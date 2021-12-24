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

QVector<BtWord> KdOnlineLDecoder::getResult(CompactLattice *out_fst,
                                            QVector<QString> lexicon)
{
    QVector<BtWord> result;
    MakeLattice(0, frame_toks.size(), out_fst);
    if( out_fst->Start() )
    {
        return result;
    }

    KdMBR *mbr = NULL;
    mbr = new KdMBR(out_fst);
    result = mbr->getResult(lexicon);

    HaveSilence();
    return result;
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

    if( 1)
    {
        status.state = KD_STATE_SILENCE; //see silence
    }
    else
    {
        status.state = KD_STATE_NORMAL;
    }

    return false; ///shit! change this to true
}

int KdOnlineLDecoder::Decode(DecodableInterface *decodable)
{
    if( status.state==KD_STATE_SILENCE )
    {
        ResetDecoder();
        qDebug() << "reset";
    }
    ProcessNonemitting(std::numeric_limits<float>::max());
    int frame_i;
    for ( frame_i=0 ; frame_i<opts_.batch_size; frame_i++)
    {
        if( frame_num>=decodable->NumFramesReady() )
        {
            break;
        }


        if ( (frame_num+1)%config_.prune_interval==0 )
        {
//            PruneActiveTokens(config_.lattice_beam * config_.prune_scale);
        }
        float weight_cutoff = ProcessEmitting(decodable);
        ProcessNonemitting(weight_cutoff);

        utt_frames_++;
        frame++;
    }
}

#endif
