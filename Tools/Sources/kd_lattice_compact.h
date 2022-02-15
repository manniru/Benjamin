#ifndef KD_LATTICE_COMPACT_H
#define KD_LATTICE_COMPACT_H

#include<QVector>
#include "kd_lattice.h"
//#include "lat/kaldi-lattice.h"



// careful: kaldi::int32 is not always the same C type as fst::int32
typedef fst::CompactLatticeWeightTpl<KdLatticeWeight, int32> CompactLatticeWeight;

typedef fst::CompactLatticeWeightCommonDivisorTpl<KdLatticeWeight, int32>
  CompactLatticeWeightCommonDivisor;
typedef fst::ArcTpl<CompactLatticeWeight> KdCompactLatticeArc;
typedef fst::VectorFst<KdCompactLatticeArc> KdCompactLattice;

/// As LatticeStateTimes, but in the KdCompactLattice format.  Note: must
/// be topologically sorted.  Returns length of the utterance in frames, which
/// might not be the same as the maximum time in the lattice, due to frames
/// in the final-prob.
int kd_compactLatticeStateTimes(KdCompactLattice &clat,
                                  std::vector<int> *times);

#endif // KD_LATTICE_COMPACT_H
