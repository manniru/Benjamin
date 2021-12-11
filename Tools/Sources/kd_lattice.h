#ifndef KD_LATTICE_H
#define KD_LATTICE_H

#include <QDebug>
#include "lat/kaldi-lattice.h"

#define KD_INVALID_STATE  -1  // Not a valid state ID.
#define KD_INVALID_ARC    -1  // Null Arc
#define KD_SHORTEST_DELTA 1e-6

typedef fst::StdFst        KdFST;
typedef fst::StdFst::Arc   KdArc;
typedef KdArc::StateId     KdStateId;

void kd_fstShortestPath(kaldi::Lattice *ifst, kaldi::Lattice *ofst);
bool SingleShortestPath(kaldi::Lattice *ifst, std::vector<kaldi::LatticeArc::Weight> *distance,
        KdStateId *f_parent, std::vector<std::pair<KdStateId, size_t>> *parent);


#endif // KD_LATTICE_H
