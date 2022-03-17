#ifndef KD_FST_UTIL_H
#define KD_FST_UTIL_H

#include <QDebug>
#include "kd_lattice.h"
#include "kd_lattice_compact.h"

bool kd_GetLinearSymbolSequence(const fst::Fst<KdArc> &fst,
                             std::vector<int> *isymbols_out,
                             std::vector<int> *osymbols_out,
                             KdArc::Weight *tot_weight_out);


KdStateId kd_CreateSuperFinal(KdCompactLattice *fst);

#endif // KD_FST_UTIL_H
