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

    CalcFinal(&result);
    HaveSilence(result);
    return result;
}

void KdOnlineLDecoder::CalcFinal(QVector<BtWord> *result)
{
    int min_diff = 30;
    int word_count = result->size();
    for( int i=0 ; i<word_count-1 ; i++ )
    {
        int f_end = floor(result->at(i).end*100);

        if( (uframe-f_end)>min_diff )
        {
            (*result)[i].is_final = 1;
        }
    }

    //for last word
    if( word_count )
    {
        BtWord last = result->last();

        if( status.word_count!=word_count )
        {
            return;
        }
        else if( status.last_word!=last.word )
        {
            return;
        }
        else if( (uframe-(status.max_frame))>min_diff )
        {
            (*result)[word_count-1].is_final = 1;
        }
//        qDebug() << "word_count" << status.max_frame << status.last_word;
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

        if( status.word_count!=word_count )
        {
            status.word_count = word_count;
            status.last_word = last.word;
            status.max_frame = last.end*100;
            status.state = KD_STATE_NORMAL;
        }
        else if( status.last_word!=last.word )
        {
            status.last_word = last.word;
            status.max_frame = last.end*100;
            status.state = KD_STATE_NORMAL;
        }
        else if( (uframe-(status.max_frame))>sil_frm )
        {
            status.state = KD_STATE_SILENCE;
        }
//        qDebug() << "word_count" << status.max_frame << status.last_word;
    }
    else if( uframe>200 )
    {
        status.state = KD_STATE_SILENCE; //reset decoder
    }
    else
    {
        status.state = KD_STATE_NORMAL; //reset decoder
    }
//    qDebug() << uframe;
}

int KdOnlineLDecoder::Decode()
{
    if( status.state==KD_STATE_SILENCE )
    {
        qDebug() << getDiffTime(start_t) << status.max_frame;
        status.word_count = -1;
        status.last_word = "";
        status.max_frame = -1;
        start_t = clock();
        ResetDecoder();
        frame_num -= 25;
    }
    ProcessNonemitting(std::numeric_limits<float>::max());
    int frame_max = decodable->NumFramesReady();
//    for ( frame=0 ; frame<opts.batch_size ; frame++ )
    while( frame_num<frame_max )
    {
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
