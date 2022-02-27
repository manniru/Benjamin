#include "kd_online_ldecoder.h"
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

void KdOnlineLDecoder::RawLattice(KdLattice *ofst)
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
                KdLatticeArc::Weight arc_w(link->graph_cost, link->acoustic_cost - cost_offset);
                KdLatticeArc arc(link->ilabel, link->olabel,arc_w, nextstate);
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

void KdOnlineLDecoder::createStates(KdLattice *ofst)
{
    int end = frame_toks.size();
    ofst->DeleteStates();

    // First create all states.
    std::vector<KdToken2*> token_list;
    for( int f=0 ; f<end ; f++ )
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

void KdOnlineLDecoder::MakeLattice(KdCompactLattice *ofst)
{
    KdLattice raw_fst;
    double lat_beam = config_.lattice_beam;
    RawLattice(&raw_fst);

//    GetRawLattice(&raw_fst);
    kd_PruneLattice(lat_beam, &raw_fst);

    kd_detLatPhonePrunedW(trans_model_,
            &raw_fst, lat_beam, ofst, config_.det_opts);

//    fst::DeterminizeLatticePhonePrunedWrapper()
}

QVector<BtWord> KdOnlineLDecoder::getResult(KdCompactLattice *out_fst)
{
    MakeLattice(out_fst);
    if( out_fst->Start() )
    {
        return result;
    }

    KdMBR *mbr = NULL;
    mbr = new KdMBR(out_fst);
    result = mbr->getResult();

    HaveSilence();
    CalcFinal();
    return result;
}

void KdOnlineLDecoder::CalcFinal()
{
    int min_diff = 15; // 150 ms
    int word_count = result.size();
    QString buf;
    for( int i=0 ; i<word_count ; i++ )
    {
        int f_end = floor(result[i].end*100);

        if( i==0 )
        {
            if( result[i].end<0.15 )
            {
                qDebug() << "$$$$$$$ skipped " << result[i].end
                         <<  result[i].word;
                continue;
            }
        }

        if( ((uframe-f_end)>min_diff) ||
            (status.state==KD_STATE_SILENCE) )
        {
            result[i].is_final = 1;
        }
        buf += result[i].word;
        buf += " ";
    }

    if( word_count )
    {
        qDebug() << ">>> " << result.last().end
                 << uframe << buf;
    }
}

// Returns "true" if the current hypothesis ends with long silence
void KdOnlineLDecoder::HaveSilence()
{
    status.state = KD_STATE_NORMAL;
    int word_count = result.size();

    if( word_count )
    {
        printAll();
//        int last_start = result.last().start*100;

//        if( status.max_frame<last_start )
//        {
//            status.max_frame = last_start;
//        }
        int end_frame = getFirstSil();

        if( end_frame!=-1 )
        {
            status.max_frame = end_frame;
            end_frame -= status.min_frame;
            float end_time = end_frame/100.0;
            float start_time = result[word_count-1].start;
            result[word_count-1].end = end_time;
            result[word_count-1].time = (start_time+end_time)/2;
            status.state = KD_STATE_SILENCE;
            qDebug() << "end_frame" << end_frame << uframe;
        }
    }
    else if( uframe>100 )
    {
        status.state = KD_STATE_NULL; //reset decoder
    }

    if( uframe>1000 ) //cut imidietly if more than 10s
    {
        status.state = KD_STATE_BLOWN; //reset decoder
        qDebug() << "We ARE BLOWN";
    }
//    qDebug() << uframe;
}

// Get first silence
int KdOnlineLDecoder::getFirstSil()
{
    int sil_count = 10; //100ms
    int end = decodable->NumFramesReady()-sil_count;
    int start = status.min_frame;
    int mean = (end + start)/2;
    int j;

    for( int i=mean ; i<end ; i++ )
    {
        if( decodable->getPhone(i)<11 )
        {
            continue; //first element should be non-sil
        }
        for( j=1 ; j<sil_count ; j++ )
        {
            if( decodable->getPhone(i+j)>10 )
            {
                break;
            }
        }

        if( decodable->getPhone(i+j)<11 )
        {
            qDebug() << "Found Silence @ " << i;
            return i+1;
        }
    }
    return -1;
}

void KdOnlineLDecoder::printAll()
{
    QString buffer;

    int start = status.min_frame;
    int end = decodable->NumFramesReady();
    for( int i=start ; i<end ; i++ )
    {
        if( decodable->getPhone(i)<11 )
        {
            buffer += "-";
        }
        else
        {
            buffer += "|";
            buffer += QString::number(decodable->getPhone(i));
        }
    }
    //UNCOM
//    qDebug() << "##" << uframe
//             << start << buffer;
}

int KdOnlineLDecoder::Decode()
{
    checkReset(); //check if need reset
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

void KdOnlineLDecoder::checkReset()
{
    if( status.state==KD_STATE_NULL ||
        status.state==KD_STATE_BLOWN  )
    {
        qDebug() << "-----------Reset Null" << getDiffTime(start_t)
                 << status.max_frame << uframe;
        frame_num -= 35;
    }
    else if( status.state==KD_STATE_SILENCE )
    {
        int diff = status.max_frame - frame_num;
        qDebug() << "------------Reset Sil" << getDiffTime(start_t)
                 << status.max_frame << diff << uframe;
        frame_num -= diff;
    }

    if( status.state!=KD_STATE_NORMAL )
    {
        status.min_frame = frame_num;
        status.max_frame = 0;
        start_t = clock();
        ResetDecoder(); // this reset uframe
    }
}
