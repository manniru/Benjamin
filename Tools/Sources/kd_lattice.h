#ifndef KD_LATTICE_H
#define KD_LATTICE_H

#include "lat/kaldi-lattice.h"

#define KD_INVALID_STATE  -1  // Not a valid state ID.
#define KD_SHORTEST_DELTA 1e-6

typedef fst::StdFst        KdFST;
typedef fst::StdFst::Arc   KdArc;
typedef KdArc::StateId     KdStateId;

void kd_fstShortestPath(kaldi::Lattice *ifst, kaldi::Lattice *ofst);


#endif // KD_LATTICE_H
