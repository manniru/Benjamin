#include "kd_online_ldecoder.h"
#include <QDebug>

using namespace kaldi;
QString dbg_times;
#define BT_MIN_SIL 9 //100ms ((x+1)*100)

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
    DeleteElems(elements.Clear()); //replaced ClearToks
    cost_offsets.clear();
    ClearActiveTokens(); ///THIS LINE SHOULD NOT EXECUTED!
    final_costs_.clear();

    KdStateId start_state = fst_->Start();
    frame_toks.resize(1);
    KdToken2 *start_tok = new KdToken2(0.0, 0.0, NULL);
    frame_toks[0].toks = start_tok;
    elements.Insert(start_state, start_tok);
    num_elements = 1;
    uframe = 0;
    ProcessNonemitting(config_.beam);
}

void KdOnlineLDecoder::RawLattice(KdLattice *ofst)
{
    int end = frame_toks.size();
    unordered_map<KdToken2*, float> final_costs;
    ComputeFinalCosts(&final_costs, NULL, NULL);
    dbg_times += " F:";
    dbg_times += getLDiffTime();
    createStates();
    *ofst = cache_fst1;
    dbg_times += " C:";
    dbg_times += getLDiffTime();

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

void KdOnlineLDecoder::createStates()
{
    int end = frame_toks.size();
    cache_fst1.DeleteStates();
    last_cache_f = 0;

    // First create all states.
    std::vector<KdToken2*> token_list;
    for( int f=last_cache_f ; f<end ; f++ )
    {
        last_cache_f++;
        TopSortTokens(frame_toks[f].toks, &token_list);

        for( size_t i=0 ; i<token_list.size() ; i++ )
        {
            if( token_list[i]!=NULL )
            {
                token_list[i]->state = cache_fst1.AddState();
            }
        }
    }

    cache_fst1.SetStart(0);// sets the start state
}

void KdOnlineLDecoder::MakeLattice(KdCompactLattice *ofst)
{
    KdLattice raw_fst;
    double lat_beam = config_.lattice_beam;
    getDiffTime(start_t);
    dbg_times += "S:";
    dbg_times += getLDiffTime();
    RawLattice(&raw_fst);
    dbg_times += " R:";
    dbg_times += getLDiffTime();

    kd_PruneLattice(lat_beam, &raw_fst);
    dbg_times += " P:";
    dbg_times += getLDiffTime();
    kd_detLatPhonePrunedW(trans_model_, &raw_fst,
                          lat_beam, ofst, config_.det_opts);
    dbg_times += " D:";
    dbg_times += getLDiffTime();
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
    int word_count = result.size();
    QString buf;
    for( int i=0 ; i<word_count ; i++ )
    {
        if( i==0 )
        {
            if( result[i].end<0.15 )
            {
                qDebug() << "$$$$$$$ skipped " << result[i].end
                         <<  result[i].word;
                continue;
            }
        }

        int f_end = result[i].end*100;
        if( (uframe-f_end)>BT_MIN_SIL )
        {
            result[i].is_final = 1;
        }
        buf += result[i].word;
        buf += "(";
        buf += QString::number(f_end);
        buf += ")";
        buf += " ";
    }

    if( word_count )
    {
        qDebug() << ">>> " << result.last().end
                 << uframe << buf << result.size();
    }
}

// Returns "true" if the current hypothesis ends with long silence
void KdOnlineLDecoder::HaveSilence()
{
    status.state = KD_STATE_NORMAL;
    int word_count = result.size();

    if( word_count )
    {
        float end_time = result.last().end;
        int diff = uframe - end_time*100;
        if( diff>9 )
        {
            status.max_frame = end_time*100;
            status.max_frame += status.min_frame;
            status.state = KD_STATE_SILENCE;
        }
    }
    else if( uframe>150 )
    {
        status.state = KD_STATE_NULL; //reset decoder
    }

    if( uframe>1000 ) //cut imidietly if more than 10s
    {
        status.state = KD_STATE_BLOWN; //reset decoder
        qDebug() << "We ARE BLOWN";
    }
}

int KdOnlineLDecoder::Decode()
{
    checkReset(); //check if need reset
    ProcessNonemitting(std::numeric_limits<float>::max());
    int frame_max = decodable->NumFramesReady();
//    for ( frame=0 ; frame<opts.batch_size ; frame++ )
    while( frame_num<frame_max )
    {
        if ( (uframe%config_.prune_interval)==0 )
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
    dbg_times += " E:";
    dbg_times += getDiffTime(start_t);
//    qDebug() << "Check Reset" << dbg_times;
    if( status.state==KD_STATE_NULL ||
        status.state==KD_STATE_BLOWN  )
    {
        qDebug() << "----------Reset Null"
                 << status.max_frame << uframe;
        frame_num -= 35;
    }
    else if( status.state==KD_STATE_SILENCE )
    {
        int diff = frame_num - status.max_frame;
        qDebug() << "------------Reset Sil"
                 << status.max_frame << diff << uframe;
        frame_num -= diff;
    }

    if( status.state!=KD_STATE_NORMAL )
    {
        status.min_frame = frame_num;
        status.max_frame = 0;
        ResetDecoder(); // this reset uframe

        cache_fst1.DeleteStates();
        last_cache_f = 0;
    }
    start_t = clock();
    dbg_times = "";
}

//sample_rate: 16000 channel: 2 chunk_size: 57344
//Check Reset " E:15ms"
//Check Reset "S:0ms F:0ms C:30ms R:27ms P:68ms D:7ms E:193ms"
//>>>  1.93 194 "delta oscar mike " 3
//Check Reset "S:0ms F:0ms C:57ms R:50ms P:126ms D:11ms E:297ms"
//>>>  2.93 294 "delta oscar mike super one " 5
//Check Reset "S:0ms F:0ms C:87ms R:78ms P:193ms D:17ms E:436ms"
//>>>  3.43 394 "delta oscar mike super one november " 6
//Check Reset "S:0ms F:0ms C:108ms R:97ms P:243ms D:24ms E:523ms"
//------------Reset Sil 343 51 394
