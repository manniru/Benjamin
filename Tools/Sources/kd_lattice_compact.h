#ifndef KD_LATTICE_COMPACT_H
#define KD_LATTICE_COMPACT_H

#include <QVector>
#include "kd_lattice.h"
#include "kd_clat_weight.h"
#include "kd_fst_util.h"

typedef fst::ArcTpl<KdCLatWeight> KdCLatArc;
typedef fst::VectorFst<KdCLatArc> KdCompactLattice;

int kd_getStartTime(std::vector<int> input);
int kd_getEndTime(std::vector<int> input);
QVector<int> kd_getTimes(KdCompactLattice &lat);
// must be topologically sorted
void kd_compactLatticeStateTimes(KdCompactLattice &clat,
                                 std::vector<int> *times);

void kd_ConvertLattice(KdCompactLattice &ifst,
        KdLattice *ofst, bool invert = true);

void kd_RemoveAlignmentsFromCompactLattice(KdCompactLattice *fst);

KdStateId kd_CreateSuperFinal(KdCompactLattice *fst);

#endif // KD_LATTICE_COMPACT_H
