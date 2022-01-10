#include "kd_online_ldecoder.h"

#ifdef BT_LAT_ONLINE
#include "base/timer.h"
#include "fstext/fstext-utils.h"
#include "hmm/hmm-utils.h"
#include <QDebug>

using namespace kaldi;

KdOnlineLDecoder::KdOnlineLDecoder(QVector<int> sil_phones,
                                   kaldi::TransitionModel &trans_model):
    trans_model_(trans_model)
{
    fst_ = kd_readDecodeGraph(BT_FST_PATH);

    opts.max_active = 7000;
    opts.lattice_beam = 6.0;

    config_ = opts;
    config_.Check();

    silence_set = sil_phones;
    uframe = 0;
    effective_beam_ = opts.beam;
    start_t = clock();
}

//frame_num would not reset
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
    uframe = 0;
    ProcessNonemitting(config_.beam);
}


void KdOnlineLDecoder::RawLattice(Lattice *ofst)
{
    int end = frame_toks.size();
    unordered_map<KdToken2*, float> final_costs;
    ComputeFinalCosts(&final_costs, NULL, NULL);
    createStates(ofst);

    // Now create all arcs.
    for( int f=0 ; f<end ; f++ )
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

void KdOnlineLDecoder::MakeLattice(CompactLattice *ofst)
{
    Lattice raw_fst;
    double lat_beam = config_.lattice_beam;
    RawLattice(&raw_fst);

//    GetRawLattice(&raw_fst);
    PruneLattice(lat_beam, &raw_fst);

    fst::DeterminizeLatticePhonePrunedWrapper(trans_model_,
            &raw_fst, lat_beam, ofst, config_.det_opts);
}

QVector<BtWord> KdOnlineLDecoder::getResult(CompactLattice *out_fst,
                                            QVector<QString> lexicon)
{
    QVector<BtWord> result;
    MakeLattice(out_fst);
    if( out_fst->Start() )
    {
        return result;
    }

    KdMBR *mbr = NULL;
    mbr = new KdMBR(out_fst);
    result = mbr->getResult(lexicon);

    HaveSilence(result);
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
void KdOnlineLDecoder::HaveSilence(QVector<BtWord> result)
{
    int sil_frm = 100;//opts_.inter_utt_sil; //50
    int word_count = result.size();

    if( word_count )
    {
        BtWord last = result.last();
        qDebug() << "word_count" << uframe-status.max_frame << uframe;

        if( status.word_count!=word_count )
        {
            status.word_count = word_count;
            status.last_word = last.word;
            status.max_frame = last.end;
            status.state = KD_STATE_NORMAL;
        }
        else if( status.last_word!=last.word )
        {
            status.last_word = last.word;
            status.max_frame = last.end*100;
            status.state = KD_STATE_NORMAL;
        }
        else if( (uframe-(status.max_frame))>150 )
        {
            status.state = KD_STATE_SILENCE;
        }
    }
    else if( uframe>200 )
    {
        status.state = KD_STATE_SILENCE; //reset decoder
    }
    else
    {
        status.state = KD_STATE_NORMAL; //reset decoder
    }
    qDebug() << uframe << frame_num-decodable->NumFramesReady();
}

int KdOnlineLDecoder::Decode()
{
    if( status.state==KD_STATE_SILENCE )
    {
        printTime(start_t);
        start_t = clock();
        ResetDecoder();
        qDebug() << "reset";
    }
    ProcessNonemitting(std::numeric_limits<float>::max());
    int frame;
    for ( frame=0 ; frame<opts.batch_size; frame++)
    {
        if( frame_num>=decodable->NumFramesReady() )
        {
            break;
        }

        if ( (frame_num%config_.prune_interval)==0 )
        {
//            PruneActiveTokens(config_.lattice_beam * config_.prune_scale);
        }
        float weight_cutoff = ProcessEmitting();
        ProcessNonemitting(weight_cutoff);

        uframe++;
    }
}

#endif
