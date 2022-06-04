#include "kd_online_ldecoder.h"
#include <QDebug>

QString dbg_times;
#define BT_MIN_SIL 14 //150ms ((x+1)*100)

KdOnlineLDecoder::KdOnlineLDecoder(kaldi::TransitionModel *trans_model)
{
    fst_graph = kd_readDecodeGraph(BT_FST_PATH);
    mbr = new KdMBR;

    opts.max_active = 7000;

    config = opts;
    t_model = trans_model;

    uframe = 0;
    effective_beam_ = opts.beam;
    start_t = clock();
    last_cache_f = 0;

    gd_file = new QFile(BT_GRAPH_PATH);
}

void KdOnlineLDecoder::RawLattice(KdLattice *ofst)
{
    float infinity = KD_INFINITY_FL;
    int end = frame_toks.size();
    createStates(ofst);

    dbg_times += " C:";
    dbg_times += getLDiffTime();

    // Now add arcs
    for( int f=0 ; f<end ; f++ )
    {
        for( KdToken *tok=frame_toks[f].tail ; tok!=NULL ; tok=tok->prev )
        {
            int len = tok->arc.length();
            for( int i=0 ; i<len ; i++ )
            {
                BtTokenArc link = tok->arc[i];
                KdStateId nextstate = tok->arc_ns[i]->m_state;

                float cost_offset = 0.0;
                if( link.ilabel!=0 ) // emitting
                {
                    KALDI_ASSERT( f<cost_offsets.length() );
                    cost_offset = cost_offsets[f];
                }
                KdLatticeWeight arc_w(link.graph_cost, link.acoustic_cost - cost_offset);
                KdLatticeArc arc(link.ilabel, link.olabel,arc_w, nextstate);
                ofst->AddArc(tok->m_state, arc);
            }

            if( f==end-1 )
            {
                //this should not be m_state
                float final_cost = fst_graph->Final(tok->state).Value();
                if( final_cost!=infinity )
                {
                    ofst->SetFinal(tok->m_state, KdLatticeWeight(final_cost, 0));
//                    ofst->SetFinal();
                }
            }
        }
    }

    ofst->SetStart(0);// sets the start state
}

void KdOnlineLDecoder::createStates(KdLattice *ofst)
{
    int end = frame_toks.size();
    last_cache_f = 0;
    ofst->DeleteStates();

    // First create all states.
    for( int f=last_cache_f ; f<end ; f++ )
    {
        last_cache_f++;

        for( KdToken *tok=frame_toks[f].head ; tok!=NULL ; tok=tok->next )
        {
            tok->m_state = ofst->AddState();
        }
    }
}

void KdOnlineLDecoder::makeNodes()
{
    QTextStream out(gd_file);
    int end = frame_toks.size();

    // First create all states.
    for( int f=0 ; f<end ; f++ )
    {
        out << "subgraph cluster_";
        out << QString::number(f);
        out << "\n{\n";
        out << "color = \"#005000\"\n";
        out << "node [color=\"#005000\"]\n";
        out << "label = \"frame: ";
        out << QString::number(f);
        out << "\";\n";
        for( KdToken *tok=frame_toks[f].head ; tok!=NULL ; tok=tok->next )
        {
            out << "\"node";
            out << QString::number(tok->tok_id);
            out << "\" [ label = \"<f0> ";
            out << QString::number(tok->tok_id);
            out << " | <f1> ";
            out << QString::number(tok->cost);
            out << " \" shape = \"record\" ];\n";
        }
        out << "}\n";
    }
}

void KdOnlineLDecoder::makeEdge()
{
    QTextStream out(gd_file);
    int end = frame_toks.size();

    // Now add arcs
    for( int f=0 ; f<end ; f++ )
    {
        for( KdToken *tok=frame_toks[f].tail ; tok!=NULL ; tok=tok->prev )
        {
            int len = tok->arc.length();
            for( int i=0 ; i<len ; i++ )
            {
                out << "\"node";
                out << QString::number(tok->tok_id);
                out << "\":f0 -> \"node";
                out << QString::number(tok->arc_ns[i]->tok_id);
                out << "\":f0;\n";
            }
        }
    }
    out << "}\n";
    gd_file->close();

    QString cmd = "dot -Tpng graph";
    cmd += QString::number(uframe);
    cmd += " > out";
    cmd += QString::number(uframe);
    cmd += ".png";
    system(cmd.toStdString().c_str());
}

void KdOnlineLDecoder::MakeGraph(int frame)
{
    QString filename = BT_GRAPH_PATH;
    filename += QString::number(frame);

    gd_file->setFileName(filename);
    if( !gd_file->open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << BT_GRAPH_PATH;
        return;
    }
    QTextStream out(gd_file);

    out << "digraph g" << "\n";
    out << "{" << "\n";
    out << "fontname=\"Helvetica,Arial,sans-serif\"" << "\n";
    out << "node [fontname=\"Helvetica,Arial,sans-serif\"]" << "\n";
    out << "edge [fontname=\"Helvetica,Arial,sans-serif\"]" << "\n";
    out << "fontsize = \"32\"\n";
    out << "label=\"frame=";
    out << QString::number(frame);
    out << "\"\n";
    out << "labelloc = \"t\"\n";
    out << "graph [ rankdir = \"LR\" ];" << "\n";
    out << "node [ fontsize = \"16\" ];" << "\n";
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
    kp.prune(&raw_fst);
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
        buf += ",";
        buf += QString::number(result[i].conf);
        buf += ")";
        buf += " ";
    }

    if( word_count )
    {
        QString msg = "<";
        msg += QString::number(wav_id+1);
        msg += "> ";
        msg += QString::number(result.last().end);
        msg += " ";
        msg += QString::number(uframe);
        msg += " " + buf;
        qDebug() << msg << result.size();
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
        if( diff>BT_MIN_SIL )
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

int KdOnlineLDecoder::Decode()
{
    checkReset(); //check if need reset
    ProcessNonemitting(std::numeric_limits<float>::max());
    uint frame_max = decodable->NumFramesReady();

    while( frame_num<frame_max )
    {
        if(  (uframe%config.prune_interval)==0 )
        {
//            PruneActiveTokens(config_.lattice_beam * config_.prune_scale);
        }
        float weight_cutoff = ProcessEmitting();
        ProcessNonemitting(weight_cutoff);

        uframe++;

        MakeGraph(uframe);
        makeNodes();
        makeEdge();
        if( uframe>3 )
        {
            exit(0);
        }
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
        qDebug() << "--Reset Sil, Max Frame: "
                 << status.max_frame << status.min_frame
                 << diff << uframe;
        frame_num -= diff;
    }

    if( status.state!=KD_STATE_NORMAL )
    {
        status.min_frame = frame_num;
        status.max_frame = 0;
        ResetDecoder(); // this reset uframe

        cache_fst1.DeleteStates();
        last_cache_f = 0;
        status.state = KD_STATE_NORMAL;
    }
    start_t = clock();
    dbg_times = "";
}
