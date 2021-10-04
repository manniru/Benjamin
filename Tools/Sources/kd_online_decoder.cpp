#include "kd_online_decoder.h"
#include "base/timer.h"
#include "fstext/fstext-utils.h"
#include "hmm/hmm-utils.h"

using namespace kaldi;

KdOnlineLDecoder::KdOnlineLDecoder(const fst::Fst<fst::StdArc> &fst,
                                   const KdOnlineLDecoderOpts &opts,
                                   const std::vector<int32> &sil_phones,
                                   const kaldi::TransitionModel &trans_model):
    LatticeFasterDecoder(fst, opts), opts_(opts),
    silence_set_(sil_phones), trans_model_(trans_model),
    max_beam_(opts.beam), effective_beam_(opts.beam),
    state_(KD_EndFeats), frame_(0), utt_frames_(0)
{
    ;
}

void KdOnlineLDecoder::ResetDecoder(bool full)
{
    DeleteElems(toks_.Clear()); //replaced ClearToks
    StateId start_state = fst_->Start();
    KALDI_ASSERT(start_state != fst::kNoStateId);
    KdToken *dummy_token = new KdToken(0.0, 0.0, NULL, NULL, NULL); //Weight was Weight::One()
    toks_.Insert(start_state, dummy_token);
    prev_immortal_tok_ = immortal_tok_ = dummy_token;
    utt_frames_ = 0;

    if( full )
    {
        frame_ = 0;
    }
}

void KdOnlineLDecoder::UpdateImmortalToken()
{
    unordered_set<KdToken*> emitting;
    for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
    {
        KdToken* tok = e->val;
        while( tok!=NULL /*&& ( tok->ForwardLinkT.ilabel==0 )*/ ) //deal with non-emitting ones ...
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
            while( ( prev_token!=NULL ) /*&& ( prev_token->ForwardLinkT.ilabel==0 )*/ )
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

bool KdOnlineLDecoder::PartialTraceback(Lattice *out_fst)
{
    UpdateImmortalToken();
    if(immortal_tok_ == prev_immortal_tok_)
    {
        return false; //no partial traceback at that point of time
    }

    GetRawLattice(out_fst);
    return true;
}

void KdOnlineLDecoder::FinishTraceBack(Lattice *out_fst)
{
    KdToken *best_tok = NULL;
    bool is_final = ReachedFinal();

    if (!is_final)
    {
        for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
        {
            if (best_tok == NULL || best_tok->tot_cost < e->val->tot_cost )
            {
                best_tok = e->val;
            }
        }
    }
    else
    {
        double best_cost = std::numeric_limits<double>::infinity();
        for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
        {
            double this_cost = e->val->tot_cost + fst_->Final(e->key).Value();
            if (this_cost != std::numeric_limits<double>::infinity() &&
                this_cost < best_cost)
            {
                best_cost = this_cost;
                best_tok = e->val;
            }
        }
    }
    GetRawLattice(out_fst);
}


void KdOnlineLDecoder::TracebackNFrames(int32 nframes, Lattice *out_fst)
{
    KdToken *best_tok = NULL;
    for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
    {
        if (best_tok == NULL || best_tok->tot_cost<e->val->tot_cost )
        {
            best_tok = e->val;
        }
    }
    if (best_tok == NULL)
    {
        out_fst->DeleteStates();
        return;
    }
    const int32 bucket_count = num_toks_/2 + 3;
    unordered_map<KdToken*, StateId> tok_map(bucket_count);

    unordered_map<KdToken*, BaseFloat> final_costs_local;

    const unordered_map<KdToken*, BaseFloat> &final_costs =
            (decoding_finalized_ ? final_costs_ : final_costs_local);
    if (!decoding_finalized_)
    {
        ComputeFinalCosts(&final_costs_local, NULL, NULL);
    }

    out_fst->SetStart(0);
    StateId cur_state;

    KdToken *tok;
    for( tok=best_tok ; (tok != NULL) && (nframes > 0) ; tok = tok->next )
    {
        cur_state = tok_map[tok];
        for ( ForwardLinkT *l = tok->links; l != NULL; l = l->next )
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
            StateId nextstate = tok_map.find(tok->links->next_tok)->second;
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
    fst::GetLinearSymbolSequence(trace, &isymbols, osymbols, oweight);

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

    ProcessNonemitting(std::numeric_limits<float>::max());
    int32 batch_frame = 0;
    Timer timer;
    double64 tstart = timer.Elapsed(), tstart_batch = tstart;
    float factor = -1;
    for ( ; !decodable->IsLastFrame(frame_ - 1) && batch_frame < opts_.batch_size;
         ++frame_, ++utt_frames_, ++batch_frame)
    {
        if (batch_frame != 0 && (batch_frame % opts_.update_interval) == 0)
        {
            // adjust the beam if needed
            float tend = timer.Elapsed();
            float elapsed = (tend - tstart) * 1000;
            // warning: hardcoded 10ms frames assumption!
            factor = elapsed / (opts_.rt_max * opts_.update_interval * 10);
            float min_factor = (opts_.rt_min / opts_.rt_max);

            if (factor > 1 || factor < min_factor)
            {
                float update_factor = (factor > 1)?
                            -std::min(opts_.beam_update * factor, opts_.max_beam_update):
                            std::min(opts_.beam_update / factor, opts_.max_beam_update);
                effective_beam_ += effective_beam_ * update_factor;
                effective_beam_ = std::min(effective_beam_, max_beam_);
            }

            tstart = tend;
        }
        if (batch_frame != 0 && (frame_ % 200) == 0)
        {
            // one log message at every 2 seconds assuming 10ms frames
            KALDI_VLOG(3) << "Beam: " << effective_beam_
                          << "; Speed: " << ((timer.Elapsed() - tstart_batch) * 1000) / (batch_frame*10)
                          << " xRT";
        }
        float weight_cutoff = ProcessEmitting(decodable);
        ProcessNonemitting(weight_cutoff);
    }
    if( batch_frame == opts_.batch_size && !decodable->IsLastFrame(frame_ - 1) )
    {
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
    }
    return state_;
}
