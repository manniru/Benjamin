#ifndef KD_LATTICE_H
#define KD_LATTICE_H

#include <QDebug>
#include "bt_config.h"
#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <util/kaldi-table.h>
#include "kd_lattice_weight.h"

#define FST_ERROR         0x4ULL
#define FST_PLUS_ZERO     0x8ULL // for semi ring null plus
#define FST_PROPERTY      0xffffffff0007ULL
#define KD_SHORTEST_DELTA 1e-6

#define KD_INVALID_STATE  -1  // Not a valid state ID.
#define KD_INVALID_ARC    -1  // Null Arc

typedef fst::StdFst        KdFST;
typedef fst::StdFst::Arc   KdArc;
typedef KdArc::StateId     KdStateId;

typedef fst::ArcTpl<KdLatticeWeight> KdLatticeArc;
typedef fst::VectorFst<KdLatticeArc> KdLattice;


//SS: Single Shortest
void kd_fstSSPathBacktrace(KdLattice *ifst, KdLattice *ofst,
    std::vector<std::pair<KdStateId, size_t>> &parent,KdStateId f_parent);
void kd_fstShortestPath(KdLattice *ifst, KdLattice *ofst);
bool kd_SingleShortestPath(KdLattice *ifst, KdStateId *f_parent,
                        std::vector<std::pair<KdStateId, size_t>> *parent);
void kd_writeLat(KdLattice *ifst);
fst::Fst<fst::StdArc> *kd_readDecodeGraph(std::string filename);



#endif // KD_LATTICE_H
