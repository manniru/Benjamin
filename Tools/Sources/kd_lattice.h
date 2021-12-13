#ifndef KD_LATTICE_H
#define KD_LATTICE_H

#include <QDebug>
#include "lat/kaldi-lattice.h"

#define FST_ERROR         0x4ULL
#define FST_PROPERTY      0xffffffff0007ULL
#define KD_INVALID_STATE  -1  // Not a valid state ID.
#define KD_INVALID_ARC    -1  // Null Arc
#define KD_SHORTEST_DELTA 1e-6

typedef fst::StdFst        KdFST;
typedef fst::StdFst::Arc   KdArc;
typedef KdArc::StateId     KdStateId;

void kd_fstShortestPath(kaldi::Lattice *ifst, kaldi::Lattice *ofst);
bool SingleShortestPath(kaldi::Lattice *ifst, KdStateId *f_parent,
                        std::vector<std::pair<KdStateId, size_t>> *parent);
//SS: Single Shortest
void kd_fstSSPathBacktrace(kaldi::Lattice *ifst, kaldi::Lattice *ofst,
    std::vector<std::pair<KdStateId, size_t>> &parent,KdStateId f_parent);


#endif // KD_LATTICE_H
