#ifndef KD_LATTICE_COMPACT_H
#define KD_LATTICE_COMPACT_H

#include<QVector>
#include "kd_lattice.h"


/// As LatticeStateTimes, but in the CompactLattice format.  Note: must
/// be topologically sorted.  Returns length of the utterance in frames, which
/// might not be the same as the maximum time in the lattice, due to frames
/// in the final-prob.
int kd_compactLatticeStateTimes(kaldi::CompactLattice &clat,
                                  std::vector<int> *times);

#endif // KD_LATTICE_COMPACT_H
