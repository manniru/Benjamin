#ifndef KD_LATTICE_H
#define KD_LATTICE_H

#include <QDebug>
#include "bt_config.h".
#include "kd_token2.h"

#define FST_ERROR         0x4ULL
#define FST_PROPERTY      0xffffffff0007ULL
#define KD_SHORTEST_DELTA 1e-6


//SS: Single Shortest
void kd_fstSSPathBacktrace(kaldi::Lattice *ifst, kaldi::Lattice *ofst,
    std::vector<std::pair<KdStateId, size_t>> &parent,KdStateId f_parent);
void kd_fstShortestPath(kaldi::Lattice *ifst, kaldi::Lattice *ofst);
bool kd_SingleShortestPath(kaldi::Lattice *ifst, KdStateId *f_parent,
                        std::vector<std::pair<KdStateId, size_t>> *parent);
void kd_writeLat(kaldi::Lattice *ifst);


#endif // KD_LATTICE_H
