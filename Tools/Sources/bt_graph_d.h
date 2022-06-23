#ifndef BT_GRAPH_D_H
#define BT_GRAPH_D_H

#include <QVector>
#include "config.h"
#include "backend.h"

// This class create hraph file from current state
// for debug purposes

#include "kd_decoder.h"
#include "kd_lattice.h"
#include "kd_lattice_functions.h"
#include "kd_mbr.h"

class BtGraphD
{
public:
    BtGraphD();

    void MakeGraph(int frame);
    void makeNodes(QVector<KdTokenList> *frame_toks);
    void makeEdge (QVector<KdTokenList> *frame_toks);

private:
    QFile     *gd_file; // graph debug
};

#endif // BT_GRAPH_D_H
