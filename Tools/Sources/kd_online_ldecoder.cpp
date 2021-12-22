#include "kd_online_ldecoder.h"

#ifdef BT_LAT_ONLINE
#include "base/timer.h"
#include "fstext/fstext-utils.h"
#include "hmm/hmm-utils.h"
#include <QDebug>

using namespace kaldi;

KdOnlineLDecoder::KdOnlineLDecoder(fst::Fst<fst::StdArc> &fst,
                                   KdOnlineLDecoderOpts &opts,
                                   QVector<int> sil_phones,
                                   kaldi::TransitionModel &trans_model):
    KdLatticeDecoder(fst, opts), opts_(opts),
    trans_model_(trans_model), max_beam_(opts.beam)
{
    silence_set = sil_phones;
    frame = 0;
    utt_frames_ = 0;
    state_ = KD_STATE_NORMAL;
    effective_beam_ = opts.beam;
}

//frame num would not reset
void KdOnlineLDecoder::ResetDecoder()
{
//    qDebug() << "Reset Kaldi" << full << frame_toks.size();
    DeleteElems(toks_.Clear()); //replaced ClearToks
    cost_offsets.clear();
    ClearActiveTokens(); ///THIS LINE SHOULD NOT EXECUTED!
    warned_ = false;
    num_toks_ = 0;
    decoding_finalized_ = false;
    final_costs_.clear();

    KdStateId start_state = fst_->Start();
    KALDI_ASSERT(start_state != fst::kNoStateId);
    frame_toks.resize(1);
    //Weight was Weight::One()
    KdToken2 *dummy_token = new KdToken2(0.0, 0.0, NULL);
    frame_toks[0].toks = dummy_token;
    toks_.Insert(start_state, dummy_token);
    num_toks_++;
    prev_immortal_tok_ = immortal_tok_ = dummy_token;
    utt_frames_ = 0;
    ProcessNonemitting(config_.beam);
}


void KdOnlineLDecoder::RawLattice(int start, int end,
                                  Lattice *ofst)
{
    unordered_map<KdToken2*, float> final_costs;
    ComputeFinalCosts(&final_costs, NULL, NULL);
    createStates(ofst);

    // Now create all arcs.
    for( int f=start ; f<end ; f++ )
    {
        for( KdToken2 *tok=frame_toks[f].toks ; tok!=NULL ; tok=tok->next )
        {
            KdStateId cur_state = tok->state;
            KdFLink *link;
            for ( link=tok->links; link!=NULL; link=link->next )
            {
                KdStateId nextstate = link->next_tok->state;

                float cost_offset = 0.0;
                if( link->ilabel!=0 ) // emitting
                {
                    KALDI_ASSERT(f >= 0 && f<cost_offsets.length() );
                    cost_offset = cost_offsets[f];
                }
                LatticeArc::Weight arc_w(link->graph_cost, link->acoustic_cost - cost_offset);
                LatticeArc arc(link->ilabel, link->olabel,arc_w, nextstate);
                ofst->AddArc(cur_state, arc);
            }
            if( f==end-1 )
            {
                if( !final_costs.empty() )
                {
                    unordered_map<KdToken2*, float>::const_iterator
                            iter = final_costs.find(tok);
                    if( iter!=final_costs.end() )
                    {
//                        LatticeWeight w = LatticeWeight(iter->second, 0);
                        ofst->SetFinal(cur_state, LatticeWeight(iter->second, 0));
                    }
                }
                else
                {
                    ofst->SetFinal(cur_state, LatticeWeight::One());
                }
            }

        }
    }
}

void KdOnlineLDecoder::createStates(Lattice *ofst)
{
    int end = frame_toks.size();
    ofst->DeleteStates();

    // First create all states.
    std::vector<KdToken2*> token_list;
    for( int32 f=0 ; f<end ; f++ )
    {
        if( frame_toks[f].toks==NULL )
        {
            KALDI_WARN << "GetRawLattice: no tokens active on frame " << f;
            return;
        }
        TopSortTokens(frame_toks[f].toks, &token_list);
//        if( dbg )
//            qDebug() << "token_list" << f << token_list.size() << start;
        for( size_t i=0 ; i<token_list.size() ; i++ )
        {
            if( token_list[i]!=NULL )
            {
                token_list[i]->state = ofst->AddState();
            }
        }
    }

    ofst->SetStart(0);// sets the start state
}

void KdOnlineLDecoder::MakeLattice(int start, int end,
                                   CompactLattice *ofst)
{
    Lattice raw_fst;
    double lat_beam = config_.lattice_beam;
    RawLattice(start, end, &raw_fst);

//    GetRawLattice(&raw_fst);
    PruneLattice(lat_beam, &raw_fst);

    fst::DeterminizeLatticePhonePrunedWrapper(trans_model_,
            &raw_fst, lat_beam, ofst, config_.det_opts);
}

//Called if decoder is not in KdDecodeState::KD_EndUtt
bool KdOnlineLDecoder::PartialTraceback(CompactLattice *out_fst)
{
//    UpdateImmortalToken();
//    if( immortal_tok_==prev_immortal_tok_ )
//    {
//        return false;
//    }

    MakeLattice(0, frame_toks.size(), out_fst);
    return true;
}

// Makes graph, by tracing back from the best currently active token
// to the last immortal token.
double KdOnlineLDecoder::FinishTraceBack(CompactLattice *out_fst)
{
//    KdToken2 *best_tok = getBestTok();
    MakeLattice(0, frame_toks.size(), out_fst);

    return 1;
}

bool KdOnlineLDecoder::GetiSymbol(Lattice *fst,
                             std::vector<int32> *isymbols_out)
{
  LatticeArc::Weight tot_weight = LatticeArc::Weight::One();
  std::vector<int32> ilabel_seq;

  KdStateId cur_state = fst->Start();
  if( cur_state ==-1 ) // Not a valid state ID.
  {
      isymbols_out->clear();
      qDebug() << "Not a valid state ID";
      return true;
  }

  int i = 0;
  while( 1 )
  {
      i++;
      LatticeArc::Weight w = fst->Final(cur_state);
      if( w!= LatticeArc::Weight::Zero() )
      {  // is final..
          qDebug() << "-----------" << i;//THIS LINE NEVER RUN!
          tot_weight = Times(w, tot_weight);
          if( fst->NumArcs(cur_state)!=0 )
          {
              return false;
          }
          *isymbols_out = ilabel_seq;
          return true;
      }
      else
      {
//          qDebug() << "Flag #2" << i;
          if( fst->NumArcs(cur_state)!=1 )
          {
              return false;
          }

          fst::ArcIterator<Lattice > iter(*fst, cur_state);  // get the only arc.
          const LatticeArc &arc = iter.Value();
          tot_weight = Times(arc.weight, tot_weight);
          if( arc.ilabel!=0 )
          {
              ilabel_seq.push_back(arc.ilabel);
          }
          cur_state = arc.nextstate;
      }
  }
}

void KdOnlineLDecoder::TracebackNFrames(int32 nframes, Lattice *out_fst)
{
    KdToken2 *best_tok = NULL;

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

    unordered_map<KdToken2*, KdStateId> tok_map(bucket_count);
    unordered_map<KdToken2*, float> final_costs_local;

    unordered_map<KdToken2*, float> final_costs;
    if (decoding_finalized_)
    {
        final_costs = final_costs_;
    }
    else
    {
        ComputeFinalCosts(&final_costs_local, NULL, NULL);
        final_costs = final_costs_local;
    }

    std::vector<KdToken2*> token_list;
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

    KdToken2 *tok;
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
            typename unordered_map<KdToken2*, KdStateId>::const_iterator
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
        typename unordered_map<KdToken2*, BaseFloat>::const_iterator
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

// Returns "true" if the current hypothesis ends with long silence
bool KdOnlineLDecoder::HaveSilence()
{
    int32 sil_frm = 100;//opts_.inter_utt_sil; //50
    int end = frame_toks.size();
    int start = 0;//end-sil_frm;

    if( sil_frm>frame_toks.size() )
    {
        return false;
    }

    Lattice raw_fst;
    RawLattice(start, end, &raw_fst);
//    kd_writeLat(&raw_fst);

//    double lat_beam = config_.lattice_beam;
//    PruneLattice(lat_beam, &raw_fst);

//    Lattice lat;
//    kd_fstShortestPath(&raw_fst, &lat);

//    std::vector<int32> isymbols;
//    GetiSymbol(&lat, &isymbols);

//    std::vector<std::vector<int32> > split;
//    SplitToPhones(trans_model_, isymbols, &split);

//    for( int i = 0; i<split.size(); i++)
//    {
//        int32 tid = split[i][0];
//        int32 phone = trans_model_.TransitionIdToPhone(tid);
//        if( !silence_set.contains(phone) )
//        {
//            qDebug() << "split" << split.size();
//            return false;
//        }
//    }

//    CompactLattice clat;
//    MakeLattice(start, end, &clat);

//    fst::CreateSuperFinal(&clat); // Add super-final state (i.e. just one final state).

//    // Topologically sort the lattice, if not already sorted.
//    kaldi::uint64 props = clat.Properties(fst::kFstProperties, false);
//    if( !(props & fst::kTopSorted) )
//    {
//        if (fst::TopSort(&clat) == false)
//            KALDI_ERR << "Cycles detected in lattice.";
//    }
    std::vector<int32> state_times_; // time of each state in the word lattice,
//    int f = CompactLatticeStateTimes(clat, &state_times_); // get times of the states
    PruneLattice(config_.lattice_beam, &raw_fst);
    kd_latticeGetTimes(&raw_fst, &state_times_);

    qDebug() << state_times_.size() << raw_fst.NumStates();// << f;

    std::vector<int32> isymbols;
    GetiSymbol(&raw_fst, &isymbols);

    return false; ///shit! change this to true
}

int KdOnlineLDecoder::Decode(DecodableInterface *decodable)
{
    if( state_==KD_STATE_SILENCE )
    {
        ResetDecoder();
        qDebug() << "reset";
    }
    checkIntegrity("(1)");
    ProcessNonemitting(std::numeric_limits<float>::max());
    checkIntegrity("(2)");
    int frame_i;
    for ( frame_i=0 ; frame_i<opts_.batch_size; frame_i++)
    {
        if( frame_num>=decodable->NumFramesReady() )
        {
            break;
        }


        if ((frame_num+1) % config_.prune_interval == 0)
        {
//            PruneActiveTokens(config_.lattice_beam * config_.prune_scale);
        }
        float weight_cutoff = ProcessEmitting(decodable);
        checkIntegrity("(3)");
        ProcessNonemitting(weight_cutoff);
        checkIntegrity("(4)");

        utt_frames_++;
        frame++;
    }

    if( HaveSilence() )
    {
        state_ = KD_STATE_SILENCE; //see silence
    }
    else
    {
        state_ = KD_STATE_NORMAL;
    }

    return state_;
}

#endif
