#include "kd_decoder.h"
#include <QDebug>

using namespace kaldi;

KdDecoder::KdDecoder()
{
    max_state = 0;
    warned_ = false;
    decoding_finalized_ = false;

    for( int i=0 ; i<MAX_STATE_COUNT ; i++ )
    {
        all_tokens[i] = NULL;
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
    KdToken *start_tok = new KdToken(0.0, 0.0);
    KdStateId start_state = fst_->Start();
    start_tok->state = start_state;
    frame_toks[0].insert(start_tok);
    all_tokens[start_state] = start_tok;
    max_state = 1;
    ProcessNonemitting(config.beam);
}

// update or inserts a new to frame_toks[frame]
// return true if a token created or cost changed.
bool KdDecoder::updateToken(KdStateId state, float tot_cost,
                            KdToken **tok)
{
    bool changed = false;
    if( all_tokens[state]==NULL )
    {
        float extra_cost = 0.0;
        KdToken *new_tok = new KdToken(tot_cost, extra_cost);
        new_tok->state = state;
        frame_toks.last().insert(new_tok);
        all_tokens[state] = new_tok;
        changed = true;
    }
    else// update old token
    {
        KdToken *tok = all_tokens[state];
        if( tok->tot_cost > tot_cost )
        {
            tok->tot_cost = tot_cost;
            // we don't allocate a new token, the old stays linked in frame_toks
            // we only replace the tot_cost
            changed = true;
        }
    }
    *tok = all_tokens[state];
    return changed;
}

// Get Cutoff and Also Update adaptive_beam
float KdDecoder::GetCutoff(KdToken **best_tok)
{
    float best_cost = std::numeric_limits<float>::infinity();
    size_t count = 0;

    std::vector<float> tmp;
    KdToken *head = frame_toks[uframe].head;
    for( KdToken *tok=head ; tok!=NULL ; tok=tok->next )
    {
        KdStateId state = tok->state;
        if( state!=-1 )
        {
            float cost = tok->tot_cost;
            tmp.push_back(cost);
            if( cost<best_cost )
            {
                best_cost = cost;
                *best_tok = tok;
            }
            count++;
        }
    }

    float beam_cutoff = best_cost + config.beam;
    float min_active_cutoff = std::numeric_limits<float>::infinity();
    float max_active_cutoff = std::numeric_limits<float>::infinity();

    KALDI_VLOG(6) << "Number of tokens active on frame " << frame_num
                  << " is " << tmp.size();

    if( tmp.size()>config.max_active )
    {
        std::nth_element(tmp.begin(),
                         tmp.begin() + config.max_active,
                         tmp.end());
        max_active_cutoff = tmp[config.max_active];
    }
    if( max_active_cutoff < beam_cutoff)
    {
        // max_active is tighter than beam.
        adaptive_beam = max_active_cutoff - best_cost + config.beam_delta;
        return max_active_cutoff;
    }
    if( tmp.size() > static_cast<size_t>(config.min_active))
    {
        if( config.min_active == 0)
        {
            min_active_cutoff = best_cost;
        }
        else
        {
            std::nth_element(tmp.begin(),
                             tmp.begin() + config.min_active,
                             tmp.size() > static_cast<size_t>(config.max_active) ?
                                 tmp.begin() + config.max_active :
                                 tmp.end());
            min_active_cutoff = tmp[config.min_active];
        }
    }
    if( min_active_cutoff > beam_cutoff)
    { // min_active is looser than beam.
        adaptive_beam = min_active_cutoff - best_cost + config.beam_delta;
        return min_active_cutoff;
    }
    else
    {
        adaptive_beam = config.beam;
        return beam_cutoff;
    }
}

double KdDecoder::GetBestCutoff(KdToken *tok)
{
    double cutoff = KD_INFINITY;
    int frame = cost_offsets.size();

    cost_offsets.push_back(0.0);

    float cost_offset = -tok->tot_cost;

    for( fst::ArcIterator<KdFST> aiter(*fst_, tok->state) ; !aiter.Done() ; aiter.Next() )
    {
        const KdArc &arc = aiter.Value();
        if( arc.ilabel!=0 )
        {
            float arc_cost = -decodable->LogLikelihood(frame_num, arc.ilabel);
            float new_weight = arc.weight.Value() + (cost_offset + tok->tot_cost)
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
            float cost = tok->tot_cost;
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
    int frame = frame_toks.size() - 2;

    for(fst::ArcIterator<KdFST> aiter(*fst_, tok->state); !aiter.Done(); aiter.Next() )
    {
        const KdArc &arc = aiter.Value();
        if( arc.ilabel!=0 )
        {
            float new_weight = decodable->LogLikelihood(frame_num, arc.ilabel);
            float ac_cost = cost_offsets[frame] - new_weight;
            float graph_cost = arc.weight.Value();
            float tot_cost = tok->tot_cost + ac_cost + graph_cost;

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

            ef_tok->ilabel = arc.ilabel;
            ef_tok->olabel = arc.olabel;
            ef_tok->graph_cost = graph_cost;
            ef_tok->acoustic_cost = ac_cost;
            // Add ForwardLink from tok to next_tok (put on head of list tok->links)
            tok->links = new KdFLink(ef_tok, arc.ilabel, arc.olabel,
                                       graph_cost , ac_cost, tok->links);
        }
    }
    return next_cutoff;
}

// Processes Single Non Emiting State
void KdDecoder::PNonemittingState(KdToken *tok, float cutoff)
{
    float cur_cost = tok->tot_cost;
    if( cur_cost>=cutoff )
    {
        return;// Don't bother processing
    }
    // If "tok" has any existing forward links, delete them,
    // because we're about to regenerate them.  This is a kind
    // of non-optimality (since most states are emitting it's not a huge issue.)
    DeleteForwardLinks(tok); // necessary when re-visiting

    for( fst::ArcIterator<KdFST> aiter(*fst_, tok->state) ; !aiter.Done() ; aiter.Next() )
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

                ef_tok->ilabel = 0;
                ef_tok->olabel = arc.olabel;
                ef_tok->graph_cost = graph_cost;
                ef_tok->acoustic_cost = 0;

                tok->links = new KdFLink(ef_tok, 0, arc.olabel,
                                         graph_cost, 0, tok->links);

                if( changed && fst_->NumInputEpsilons(arc.nextstate) != 0)
                {
                    PNonemittingState(ef_tok, cutoff);
                }
            }
        }
    }
}

// Deletes the elements of the singly linked list tok->links.
void KdDecoder::DeleteForwardLinks(KdToken *tok)
{
    KdFLink *l = tok->links;
    KdFLink *m;
    while( l!=NULL )
    {
        m = l->next;
        delete l;
        l = m;
    }
    tok->links = NULL;
}

// The KdToken pointers
void KdDecoder::ClaerAllToks()
{
    for( KdStateId state=1 ; state<MAX_STATE_COUNT ; state++ )
    {
        if( all_tokens[state]!=NULL )
        {
            all_tokens[state] = NULL;
        }
    }
}

void KdDecoder::ClearActiveTokens()
{
    for (size_t i=0 ; i<frame_toks.size() ; i++)
    {
        KdToken *tok=frame_toks[i].head;
        while( tok!=NULL )
        {
            DeleteForwardLinks(tok);
            KdToken *next_tok = tok->next;
            delete tok;
            max_state--;
            tok = next_tok;
        }
    }
    frame_toks.clear();
}
