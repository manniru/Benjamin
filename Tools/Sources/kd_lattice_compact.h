#ifndef KD_LATTICE_COMPACT_H
#define KD_LATTICE_COMPACT_H

#include<QVector>
#include "kd_lattice.h"
#include "kd_clat_weight.h"

typedef fst::ArcTpl<KdCLatWeight> KdCLatArc;
typedef fst::VectorFst<KdCLatArc> KdCompactLattice;

int kd_getStartTime(std::vector<int> input);
int kd_getEndTime(std::vector<int> input);
QVector<int> kd_getTimes(KdCompactLattice &lat);
// must be topologically sorted
void kd_compactLatticeStateTimes(KdCompactLattice &clat,
                                 std::vector<int> *times);


void ConvertLattice(const KdLattice &ifst,
                    KdCompactLattice *ofst, bool invert);


void RemoveAlignmentsFromCompactLattice(KdCompactLattice *fst)
{
    typedef fst::MutableFst<KdCLatArc> Fst;
    KdStateId num_states = fst->NumStates();
    for( KdStateId s=0 ; s<num_states ; s++ )
    {
        for (fst::MutableArcIterator<Fst> aiter(fst, s);
             !aiter.Done(); aiter.Next())
        {
            KdCLatArc arc = aiter.Value();
            arc.weight = KdCLatWeight(arc.weight.Weight(), std::vector<int>());
            aiter.SetValue(arc);
        }
        KdCLatWeight final_weight = fst->Final(s);
        if (final_weight != KdCLatWeight::Zero())
        {
            fst->SetFinal(s, KdCLatWeight(final_weight.Weight(), std::vector<int>()));
        }
    }
}

#endif // KD_LATTICE_COMPACT_H
