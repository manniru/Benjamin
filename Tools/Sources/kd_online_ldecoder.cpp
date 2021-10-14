#include "kd_online_ldecoder.h"

#ifdef BT_LAT_ONLINE
#include "base/timer.h"
#include "fstext/fstext-utils.h"
#include "hmm/hmm-utils.h"
#include <QDebug>

using namespace kaldi;

KdOnlineLDecoder::KdOnlineLDecoder(fst::Fst<fst::StdArc> &fst,
                                   KdOnlineLDecoderOpts &opts,
                                   std::vector<int32> &sil_phones,
                                   kaldi::TransitionModel &trans_model):
    KdLatticeDecoder(fst, opts), opts_(opts),
    silence_set_(sil_phones), trans_model_(trans_model),
    max_beam_(opts.beam), effective_beam_(opts.beam),
    state_(KD_EndFeats), frame_(0), utt_frames_(0)
{
    ;
}

void KdOnlineLDecoder::ResetDecoder(bool full)
{
//    qDebug() << "Reset Kaldi" << full << frame_toks.size();
    DeleteElems(toks_.Clear()); //replaced ClearToks
    cost_offsets_.clear();
    ClearActiveTokens(); ///THIS LINE SHOULD NOT EXECUTED!
    warned_ = false;
    num_toks_ = 0;
    decoding_finalized_ = false;
    final_costs_.clear();

    KdStateId start_state = fst_->Start();
    KALDI_ASSERT(start_state != fst::kNoStateId);
    frame_toks.resize(1);
    //Weight was Weight::One()
    KdToken *dummy_token = new KdToken(0.0, 0.0, NULL, NULL, NULL);
    frame_toks[0].toks = dummy_token;
    toks_.Insert(start_state, dummy_token);
    num_toks_++;
    prev_immortal_tok_ = immortal_tok_ = dummy_token;
    utt_frames_ = 0;
    ProcessNonemitting(config_.beam);

    if( full )
    {
        frame_ = 0;
    }
}

void KdOnlineLDecoder::MakeLattice(CompactLattice *ofst)
{
    Lattice raw_fst;
    GetRawLattice(&raw_fst);
    Invert(&raw_fst);
    fst::ILabelCompare<LatticeArc> ilabel_comp;
    ArcSort(&raw_fst, ilabel_comp);

    fst::DeterminizeLatticePrunedOptions lat_opts;
    lat_opts.max_mem = config_.det_opts.max_mem;

    ConvertLattice(raw_fst, ofst);
    DeterminizeLatticePruned(raw_fst, config_.lattice_beam, ofst, lat_opts);

    raw_fst.DeleteStates();  // Free memory-- raw_fst no longer needed.
    Connect(ofst); //removing states and arcs that are not on successful paths.
}

void KdOnlineLDecoder::UpdateImmortalToken()
{
    unordered_set<KdToken*> emitting;
    for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
    {
        KdToken* tok = e->val;
        while( tok!=NULL /*&& ( tok->KdFLink.ilabel==0 )*/ ) //deal with non-emitting ones ...
        {
            tok = tok->next;
        }
        if (tok != NULL)
        {
            emitting.insert(tok);
        }
    }

    KdToken* the_one = NULL;
    while (1)
    {
        if (emitting.size() == 1)
        {
            the_one = *(emitting.begin());
            break;
        }

        if (emitting.size() == 0)
        {
            break;
        }

        unordered_set<KdToken*> prev_emitting;
        unordered_set<KdToken*>::iterator it;

        for( it=emitting.begin(); it!=emitting.end() ; ++it )
        {
            KdToken *tok = *it;
            KdToken *prev_token = tok->next;
            while( ( prev_token!=NULL ) /*&& ( prev_token->KdFLink.ilabel==0 )*/ )
            {
                prev_token = prev_token->next; //deal with non-emitting ones
            }
            if (prev_token == NULL)
            {
                continue;
            }
            prev_emitting.insert(prev_token);
        } // for
        emitting = prev_emitting;
    } // while
    if (the_one != NULL)
    {
        prev_immortal_tok_ = immortal_tok_;
        immortal_tok_ = the_one;
        return;
    }
}

//Called if decoder is not in KdDecodeState::KD_EndUtt
bool KdOnlineLDecoder::PartialTraceback(CompactLattice *out_fst)
{
//    UpdateImmortalToken();
//    if(immortal_tok_ == prev_immortal_tok_)
//    {
//        return false; //no partial traceback at that point of time
//    }

    MakeLattice(out_fst);
    return true;
}

double KdOnlineLDecoder::FinishTraceBack(CompactLattice *out_fst)
{
    MakeLattice(out_fst);

    return 1;
}

void KdOnlineLDecoder::TracebackNFrames(int32 nframes, Lattice *out_fst)
{
    KdToken *best_tok = NULL;

    //Find token with the maximum cost->best_tok
    for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
    {
        float elem_cost = e->val->tot_cost;
        if( best_tok==NULL || best_tok->tot_cost<elem_cost )
        {
            best_tok = e->val;
        }
    }
    if (best_tok == NULL)
    {
        out_fst->DeleteStates();
        return;
    }

    int active_count = frame_toks.size();
    int bucket_count = num_toks_/2 + 3;

    unordered_map<KdToken*, KdStateId> tok_map(bucket_count);
    unordered_map<KdToken*, BaseFloat> final_costs_local;

    unordered_map<KdToken*, BaseFloat> final_costs;
    if (decoding_finalized_)
    {
        final_costs = final_costs_;
    }
    else
    {
        ComputeFinalCosts(&final_costs_local, NULL, NULL);
        final_costs = final_costs_local;
    }

    std::vector<KdToken*> token_list;
    for ( int i=0; i<active_count; i++)
    {
        if (frame_toks[i].toks == NULL)
        {
            return;
        }
        TopSortTokens(frame_toks[i].toks, &token_list);

        for( int j=0; j<token_list.size() ; j++ )
        {
            if( token_list[j]!=NULL )
            {
                tok_map[token_list[j]] = out_fst->AddState();
            }
        }
    }

    out_fst->SetStart(0);
    KdStateId cur_state;

    KdToken *tok;
    for( tok=best_tok ; (tok!=NULL) ; tok = tok->next )
    {
        cur_state = tok_map[tok];
        for ( KdFLink *l = tok->links; (l!=NULL) && (nframes>0); l=l->next )
        {
            if (l->ilabel != 0) // count only the non-epsilon arcs
            {
                --nframes;
            }

            float next_cost = 0;
            if ( tok->next )
            {
                next_cost = tok->next->tot_cost;
            }

            float tot_cost = tok->tot_cost - next_cost;
            float graph_cost = l->graph_cost;
            float ac_cost = tot_cost - graph_cost;
            typename unordered_map<KdToken*, KdStateId>::const_iterator
                    iter = tok_map.find(l->next_tok);
            KdStateId nextstate = iter->second;
            LatticeArc larc(l->ilabel, l->olabel,
                            LatticeWeight(graph_cost, ac_cost), nextstate);

            larc.nextstate = out_fst->AddState();
            out_fst->AddArc(cur_state, larc);
        }
    }

    if (!final_costs.empty())
    {
        typename unordered_map<KdToken*, BaseFloat>::const_iterator
                iter = final_costs.find(tok);
        if (iter != final_costs.end())
            out_fst->SetFinal(cur_state, LatticeWeight(iter->second, 0));
    }
    else
    {
        out_fst->SetFinal(cur_state, LatticeWeight::One());
    }
    RemoveEpsLocal(out_fst);

}

bool KdOnlineLDecoder::HaveSilence()
{
    Lattice trace;
    int32 sil_frm = opts_.inter_utt_sil / (1 + utt_frames_ / opts_.max_utt_len_);
    TracebackNFrames(sil_frm, &trace);
    std::vector<int32> isymbols;
    std::vector<int32> *osymbols = NULL;
    LatticeArc::Weight *oweight = NULL;

    Lattice lat;
    ShortestPath(trace, &lat);
    fst::GetLinearSymbolSequence(lat, &isymbols, osymbols, oweight);

    std::vector<std::vector<int32> > split;
    SplitToPhones(trans_model_, isymbols, &split);

    for( int i = 0; i<split.size(); i++)
    {
        int32 tid = split[i][0];
        int32 phone = trans_model_.TransitionIdToPhone(tid);
        if (silence_set_.count(phone) == 0)
        {
            return false;
        }
    }

    return true;
}

KdDecodeState KdOnlineLDecoder::Decode(DecodableInterface *decodable)
{
    if( state_==KD_EndFeats || state_==KD_EndUtt ) // new utterance
    {
        ResetDecoder(state_ == KD_EndFeats);
    }

//    ProcessNonemitting(std::numeric_limits<float>::max());
    int frame_i;
    Timer timer;
    double64 tstart = timer.Elapsed();
    float factor = -1;
    for ( frame_i=0 ; frame_i<opts_.batch_size; frame_i++)
    {
        if( decodable->IsLastFrame(decoded_frame_i-1) )
        {
            break;
        }

        if( frame_i>0 && (frame_i%opts_.update_interval)==0 )
        {
            // adjust the beam if needed
            float tend = timer.Elapsed();
            float elapsed = (tend - tstart) * 1000;
            // warning: hardcoded 10ms frames assumption!
            factor = elapsed / (opts_.rt_max * opts_.update_interval * 10);
            float min_factor = (opts_.rt_min / opts_.rt_max);

            if (factor > 1 || factor < min_factor)
            {
                float update_factor;
                if (factor > 1)
                {
                    update_factor = -std::min(opts_.beam_update * factor, opts_.max_beam_update);
                }
                else
                {
                    update_factor = std::min(opts_.beam_update / factor, opts_.max_beam_update);
                }
                effective_beam_ += effective_beam_ * update_factor;
                effective_beam_ = std::min(effective_beam_, max_beam_);
            }

            tstart = tend;
        }

        if ((NumFramesDecoded()+1) % config_.prune_interval == 0)
        {
            PruneActiveTokens(config_.lattice_beam * config_.prune_scale);
        }
        float weight_cutoff = ProcessEmitting(decodable);
        ProcessNonemitting(weight_cutoff);

        utt_frames_++;
        frame_++;
    }
    if( frame_i == opts_.batch_size && !decodable->IsLastFrame(frame_ - 1) )
    {
        //FinalizeDecoding();
        if( HaveSilence() )
        {
            state_ = KD_EndUtt; //End of Utterance
        }
        else
        {
            state_ = KD_EndBatch;
        }
    }
    else
    {
        state_ = KD_EndFeats;
        qDebug() << "we are here";
    }
    return state_;
}

#endif
