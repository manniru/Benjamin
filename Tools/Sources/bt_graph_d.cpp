#include "bt_graph_d.h"
#include <QDebug>

BtGraphD::BtGraphD()
{
    gd_file = new QFile;
}

void BtGraphD::MakeGraph(int frame)
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

void BtGraphD::makeNodes(QVector<KdTokenList> *frame_toks)
{
    QTextStream out(gd_file);
    int end = frame_toks->size();

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
        for( KdToken *tok=(*frame_toks)[f].head ; tok!=NULL ; tok=tok->next )
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

void BtGraphD::makeEdge(QVector<KdTokenList> *frame_toks)
{
    QTextStream out(gd_file);
    int end = frame_toks->size()-1;

    // Now add arcs
    for( int f=0 ; f<end ; f++ )
    {
        for( KdToken *tok=(*frame_toks)[f].tail ; tok!=NULL ; tok=tok->prev )
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
    cmd += QString::number(end);
    cmd += " > out";
    cmd += QString::number(end);
    cmd += ".png";
    system(cmd.toStdString().c_str());
}
