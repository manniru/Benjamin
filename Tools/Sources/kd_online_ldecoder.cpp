#include "kd_online_ldecoder.h"
#include <QDebug>

QString dbg_times;

KdOnlineLDecoder::KdOnlineLDecoder(KdTransitionModel *trans_model,
                                   BtState *state): KdDecoder(state)
{
    fst_graph = kd_readDecodeGraph(state->fst_path);

    qDebug() << "Graph States Count" << kd_NumOfStates(fst_graph);
    mbr = new KdMBR(state);

    config = opts;
    t_model = trans_model;

    uframe = 0;
    effective_beam_ = opts.beam;
    start_t = clock();
    last_cache_f = 0;

    graph = new BtGraphD;
}

void KdOnlineLDecoder::RawLattice(KdLattice *ofst)
{
    int end = frame_toks.size();
//    cache_fst1.DeleteStates();
//    last_cache_f = 0;
    createStates(last_cache_f, end);

    dbg_times += " C:";
    dbg_times += getLDiffTime();

    if( last_cache_f>0 )
    {
        last_cache_f--;
        int f = last_cache_f;
        for( KdToken *tok=frame_toks[f].head ; tok!=NULL ; tok=tok->next )
        {
            if( tok->m_state==KD_INVALID_STATE )
            {
                tok->m_state = cache_fst1.AddState();
            }
        }
    }

    // Now add arcs
    for( int f=last_cache_f ; f<end-1 ; f++ )
    {
        for( KdToken *tok=frame_toks[f].tail ; tok!=NULL ; tok=tok->prev )
        {
            int len = tok->arc.length();
            for( int i=0 ; i<len ; i++ )
            {
                BtTokenArc link = tok->arc[i];
                KdStateId nextstate = tok->arc_ns[i]->m_state;

                KdLatticeWeight arc_w(link.graph_cost, link.acoustic_cost);
                KdLatticeArc arc(link.ilabel, link.olabel,arc_w, nextstate);
                cache_fst1.AddArc(tok->m_state, arc);
            }
        }
    }

    cache_fst1.SetStart(0);// sets the start state
    last_cache_f = end;

    dbg_times += " A:";
    dbg_times += getLDiffTime();
    addFinalFrame(ofst);
}

void KdOnlineLDecoder::createStates(int start, int end)
{
    // First create all states.
    for( int f=start ; f<end ; f++ )
    {
        for( KdToken *tok=frame_toks[f].head ; tok!=NULL ; tok=tok->next )
        {
            tok->m_state = cache_fst1.AddState();
        }
    }
}

void KdOnlineLDecoder::addFinalFrame(KdLattice *ofst)
{
    float infinity = KD_INFINITY_FL;
    int lase_i = frame_toks.size()-1;
    *ofst = cache_fst1;

    for( KdToken *tok=frame_toks[lase_i].tail ; tok!=NULL ; tok=tok->prev )
    {
        int len = tok->arc.length();
        for( int i=0 ; i<len ; i++ )
        {
            BtTokenArc link = tok->arc[i];
            KdStateId nextstate = tok->arc_ns[i]->m_state;

            KdLatticeWeight arc_w(link.graph_cost, link.acoustic_cost);
            KdLatticeArc arc(link.ilabel, link.olabel,arc_w, nextstate);
            ofst->AddArc(tok->m_state, arc);
        }

        //this should not be m_state
        float final_cost = fst_graph->Final(tok->g_state).Value();
        if( final_cost!=infinity )
        {
            ofst->SetFinal(tok->m_state, KdLatticeWeight(final_cost, 0));
        }
    }
}

void KdOnlineLDecoder::printLog()
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
        buf += result[i].word;
        buf += "(";
        buf += QString::number(f_end);
        buf += ",";
        buf += QString::number(result[i].conf);
        buf += ")";
        buf += " ";
    }

    if( word_count )
    {
        char msg[300];
        sprintf(msg, "<%d> le:%.0lf uf:%d", wav_id+1,
                floor(result.last().end*100), uframe);
        qDebug() << "msg" << msg << buf << result.size();
    }

}

void KdOnlineLDecoder::MakeLattice(KdCompactLattice *ofst)
{
    KdLattice raw_fst;
    double lat_beam = KD_LAT_BEAM;
    getDiffTime(start_t);
    dbg_times += "S:";
    dbg_times += getLDiffTime();
    RawLattice(&raw_fst);
    dbg_times += " R:";
    dbg_times += getLDiffTime();

    KdPrune kp;
    dbg_times += kp.prune(&raw_fst);
    dbg_times += " P:";
    dbg_times += getLDiffTime();
    kd_detLatPhonePrunedW(t_model, &raw_fst,
                          lat_beam, ofst, config.det_opts);
    dbg_times += " D:";
    dbg_times += getLDiffTime();
}

QVector<BtWord> KdOnlineLDecoder::getResult(KdCompactLattice *out_fst)
{
    // out_fst will be reset in MakeLattice
    result.clear();
    MakeLattice(out_fst);
    if( out_fst->Start() )
    {
        return result;
    }

    mbr->compute(out_fst);
    result = mbr->getResult();

    HaveSilence();
    CalcFinal();
    return result;
}

void KdOnlineLDecoder::CalcFinal()
{
    int word_count = result.size();
    for( int i=0 ; i<word_count ; i++ )
    {
        if( i==0 )
        {
            if( result[i].end<0.15 )
            {
                continue;
            }
        }

        int f_end = result[i].end*100;
        if( (uframe-f_end)>st->min_sil )
        {
            result[i].is_final = 1;
        }
    }

    printLog();
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
        if( diff>st->min_sil )
        {
            qDebug() << "DETECT MIN SIL: "
                     << end_time*100;
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

void KdOnlineLDecoder::Decode()
{
    checkReset(); //check if need reset
    ProcessNonemitting(std::numeric_limits<float>::max());
    uint frame_max = decodable->NumFramesReady();

    while( frame_num<frame_max )
    {
//        if(  (uframe%config.prune_interval)==0 )
//        {
//            PruneActiveTokens(config_.lattice_beam * config_.prune_scale);
//        }
        float weight_cutoff = ProcessEmitting();
        ProcessNonemitting(weight_cutoff);

        uframe++;

//        graph->MakeGraph(uframe);
//        graph->makeNodes(&frame_toks);
//        graph->makeEdge(&frame_toks);
//        if( uframe>3 )
//        {
//            exit(0);
//        }
    }
}

void KdOnlineLDecoder::resetODecoder()
{
    status.min_frame = frame_num;
    status.max_frame = 0;
    ResetDecoder(); // this reset uframe

    cache_fst1.DeleteStates();
    last_cache_f = 0;
    status.state = KD_STATE_NORMAL;
}

void KdOnlineLDecoder::checkReset()
{
    dbg_times += " E:";
    dbg_times += getDiffTime(start_t);
//    qDebug() << "Reset Succ" << dbg_times;
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
        qDebug() << "--Reset Sil, Max Frame: "
                 << status.max_frame << status.min_frame
                 << diff << uframe;
        frame_num -= diff;
//        qDebug() << "Reset Succ" << dbg_times;
    }

    if( status.state!=KD_STATE_NORMAL )
    {
        resetODecoder();
    }
    start_t = clock();
    dbg_times = "";
}
