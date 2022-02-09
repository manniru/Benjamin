#ifndef KD_LATTICE_H
#define KD_LATTICE_H

#include <QDebug>
#include "bt_config.h".
#include "kd_token2.h"

#define FST_ERROR         0x4ULL
#define FST_PROPERTY      0xffffffff0007ULL
#define KD_SHORTEST_DELTA 1e-6

typedef fst::LatticeWeightTpl<float> KdLatticeWeight;
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
