#ifndef KD_LATTICE_COMPACT_H
#define KD_LATTICE_COMPACT_H

#include<QVector>
#include "kd_lattice.h"

typedef fst::CompactLatticeWeightTpl<KdLatticeWeight, int32> CompactLatticeWeight;

typedef fst::CompactLatticeWeightCommonDivisorTpl<KdLatticeWeight, int32>
  CompactLatticeWeightCommonDivisor;
typedef fst::ArcTpl<CompactLatticeWeight> KdCompactLatticeArc;
typedef fst::VectorFst<KdCompactLatticeArc> KdCompactLattice;

int kd_getStartTime(std::vector<int> input);
int kd_getEndTime(std::vector<int> input);
QVector<int> kd_getTimes(KdCompactLattice &lat);
// must be topologically sorted
void kd_compactLatticeStateTimes(KdCompactLattice &clat,
                                  std::vector<int> *times);

#endif // KD_LATTICE_COMPACT_H
