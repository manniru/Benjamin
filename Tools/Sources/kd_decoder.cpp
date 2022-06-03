#include "kd_decoder.h"
#include <QDebug>

KdDecoder::KdDecoder()
{
    max_state = 0;
    warned_ = false;
    decoding_finalized_ = false;

    for( int i=0 ; i<MAX_STATE_COUNT ; i++ )
    {
        cf_tokens[i] = NULL;
    }
}

KdDecoder::~KdDecoder()
{
    ClaerAllToks();
    ClearActiveTokens();
}

//frame_num would not reset
void KdDecoder::ResetDecoder()
{
    ClaerAllToks();
    InitDecoding(decodable);
}

void KdDecoder::InitDecoding(KdDecodable *dcodable)
{
    // clean up from last time:
    uframe = 0;
    decodable = dcodable;
    cost_offsets.clear();
    ClearActiveTokens();
    frame_toks.resize(1);
    KdToken *start_tok = new KdToken(0.0);
    KdStateId start_state = fst_->Start();
    start_tok->state = start_state;
    frame_toks[0].insert(start_tok);
    cf_tokens[start_state] = start_tok;
    max_state = 1;
    ProcessNonemitting(config.beam);
}

// update or inserts a new to frame_toks[frame]
// return true if a token created or cost changed.
bool KdDecoder::updateToken(KdStateId state, float tot_cost,
                            KdToken **tok)
{
    bool changed = false;
    if( cf_tokens[state]==NULL )
    {
        KdToken *new_tok = new KdToken(tot_cost);
        new_tok->state = state;
        frame_toks.last().insert(new_tok);
        cf_tokens[state] = new_tok;
        changed = true;
    }
    else// update old token
    {
        KdToken *tok = cf_tokens[state];
        if( tok->cost > tot_cost )
        {
            tok->cost = tot_cost;
            // we don't allocate a new token, the old stays linked in
            // frame_toks we only replace the tot_cost
            changed = true;
        }
    }
    *tok = cf_tokens[state];
    return changed;
}

// Get Cutoff and Also Update adaptive_beam
float KdDecoder::GetCutoff(KdToken **best_tok)
{
    float best_cost = KD_INFINITY_FL;

    std::vector<float> cf_costs; //current frame cost
    KdToken *head = frame_toks[uframe].head;
    for( KdToken *tok=head ; tok!=NULL ; tok=tok->next )
    {
        KdStateId state = tok->state;
        if( state!=-1 )
        {
            float cost = tok->cost;
            cf_costs.push_back(cost);
            if( cost<best_cost )
            {
                best_cost = cost;
                *best_tok = tok;
            }
        }
    }

    float beam_cutoff = best_cost + config.beam;

    int tok_count = cf_costs.size();
    if( tok_count>config.min_active )
    {
        //sub sort to nth element
        std::nth_element(cf_costs.begin(),
                         cf_costs.begin() + config.min_active,
                         cf_costs.end());
        float min_active_cutoff = cf_costs[config.min_active];

        if( min_active_cutoff>beam_cutoff )
        { // min_active is looser than beam.
            adaptive_beam = min_active_cutoff - best_cost + config.beam_delta;
            return min_active_cutoff;
        }
        else
        {
            return beam_cutoff;
        }
    }
    else
    {
        adaptive_beam = config.beam;
        return beam_cutoff;
    }
}

double KdDecoder::GetBestCutoff(KdToken *tok)
{
    double cutoff = KD_INFINITY_DB;
    if( tok==NULL )
    {
         return cutoff;
    }

    int frame = cost_offsets.size();

    cost_offsets.push_back(0.0);

    float cost_offset = -tok->cost;

    for( fst::ArcIterator<KdFST> aiter(*fst_, tok->state) ;
         !aiter.Done() ; aiter.Next() )
    {
        const KdArc &arc = aiter.Value();
        if( arc.ilabel!=0 )
        {
            float arc_cost = -decodable->LogLikelihood(frame_num, arc.ilabel);
            float new_weight = arc.weight.Value() + (cost_offset + tok->cost)
                    + arc_cost + adaptive_beam;
            if( new_weight<cutoff )
            {
                cutoff = new_weight;
            }
        }
    }
    cost_offsets[frame] = cost_offset;

    return cutoff;
}

// Processes for one frame.
float KdDecoder::ProcessEmitting()
{
    frame_toks.push_back(KdTokenList()); //add new frame tok

    KdToken *best_tok = NULL;//fst_->Start();

    float cutoff = GetCutoff(&best_tok);
    float next_cutoff = GetBestCutoff(best_tok);
    ClaerAllToks();
    KdToken *head = frame_toks[uframe].head;

    for( KdToken *tok=head ; tok!=NULL ; tok=tok->next )
    {
        KdStateId state = tok->state;
        if( state!=-1 )
        {
            float cost = tok->cost;
            if( cost<=cutoff )
            {
                next_cutoff = PEmittingState(tok, next_cutoff);
            }
        }
    }

    frame_num++;
    return next_cutoff;
}

// Processes for one frame.
void KdDecoder::ProcessNonemitting(float cutoff)
{
    // need for reverse
    KdToken *head = frame_toks.last().head;

    for( KdToken *tok=head ; tok!=NULL ; tok=tok->next )
    {
        KdStateId state = tok->state;
        if( state!=-1 )
        {
            if( fst_->NumInputEpsilons(state)!=0 )
            {
                PNonemittingState(tok, cutoff);
            }
        }
    }
}

// Processes Single Emiting State
float KdDecoder::PEmittingState(KdToken *tok, float next_cutoff)
{
    int frame = frame_toks.size() - 2; // Frame only used for c_offset
    float c_offset = cost_offsets[frame];

    for( fst::ArcIterator<KdFST> aiter(*fst_, tok->state) ;
        !aiter.Done() ; aiter.Next() )
    {
        const KdArc &arc = aiter.Value();
        if( arc.ilabel!=0 )
        {
            float new_weight = decodable->LogLikelihood(frame_num, arc.ilabel);
            float ac_cost = c_offset - new_weight;
            float graph_cost = arc.weight.Value();
            float tot_cost = tok->cost + ac_cost + graph_cost;

            if( tot_cost>=next_cutoff )
            {
                continue;
            }
            else if( tot_cost+adaptive_beam<next_cutoff )
            {
                // prune by best current token
                next_cutoff = tot_cost + adaptive_beam;
            }

            KdToken *ef_tok;
            updateToken(arc.nextstate, tot_cost, &ef_tok);

            // Add ForwardLink from tok to next_tok
            BtTokenArc n_arc; // new arc
            n_arc.ilabel = arc.ilabel;
            n_arc.olabel = arc.olabel;
            n_arc.graph_cost    = graph_cost;
            n_arc.acoustic_cost = ac_cost;

            tok->arc.push_back(n_arc);
            tok->arc_ns.push_back(ef_tok);
        }
    }
    return next_cutoff;
}

// Processes Single Non Emiting State
void KdDecoder::PNonemittingState(KdToken *tok, float cutoff)
{
    float cur_cost = tok->cost;
    if( cur_cost>=cutoff )
    {
        return;// Don't bother processing
    }
    // delete state existing arcs, because we're about to regenerate them.
    // This is non-optimality but since most states are emitting
    // it's not a huge issue.
    DeleteTokArcs(tok); // necessary when re-visiting

    for( fst::ArcIterator<KdFST> aiter(*fst_, tok->state)
         ; !aiter.Done() ; aiter.Next() )
    {
        const KdArc &arc = aiter.Value();
        if( arc.ilabel==0 ) // nonemitting
        {
            float graph_cost = arc.weight.Value();
            float tot_cost = cur_cost + graph_cost;
            if( tot_cost<cutoff )
            {
                KdToken *ef_tok;
                bool changed = updateToken(arc.nextstate, tot_cost,
                                             &ef_tok);


                // Add ForwardLink from tok to next_tok
                BtTokenArc n_arc; // new arc
                n_arc.ilabel = 0;
                n_arc.olabel = arc.olabel;
                n_arc.graph_cost    = graph_cost;
                n_arc.acoustic_cost = 0;

                tok->arc.push_back(n_arc);
                tok->arc_ns.push_back(ef_tok);

                if( changed && fst_->NumInputEpsilons(arc.nextstate)!=0)
                {
                    PNonemittingState(ef_tok, cutoff);
                }
            }
        }
    }
}

void KdDecoder::DeleteTokArcs(KdToken *tok)
{
    tok->arc.clear();
    tok->arc_ns.clear();
}

// The KdToken pointers
void KdDecoder::ClaerAllToks()
{
    for( KdStateId state=0 ; state<MAX_STATE_COUNT ; state++ )
    {
        if( cf_tokens[state]!=NULL )
        {
            cf_tokens[state] = NULL;
        }
    }
}

void KdDecoder::ClearActiveTokens()
{
    for( int i=0 ; i<frame_toks.size() ; i++ )
    {
        KdToken *tok=frame_toks[i].head;
        while( tok!=NULL )
        {
            DeleteTokArcs(tok);
            KdToken *next_tok = tok->next;
            delete tok;
            tok = next_tok;
        }
    }
    frame_toks.clear();
}
