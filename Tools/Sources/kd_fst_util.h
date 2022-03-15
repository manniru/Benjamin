#ifndef KD_FST_UTIL_H
#define KD_FST_UTIL_H

#include <QDebug>
#include "kd_lattice.h"
#include "kd_lattice_compact.h"

bool GetLinearSymbolSequence(const fst::Fst<KdArc> &fst,
                             std::vector<int> *isymbols_out,
                             std::vector<int> *osymbols_out,
                             KdArc::Weight *tot_weight_out)
{
    KdArc::Weight tot_weight = KdArc::Weight::One();
    std::vector<int> ilabel_seq;
    std::vector<int> olabel_seq;

    KdStateId cur_state = fst.Start();
    if (cur_state == fst::kNoStateId)
    {  // empty sequence.
        if (isymbols_out != NULL)
            isymbols_out->clear();
        if (osymbols_out != NULL)
            osymbols_out->clear();
        if (tot_weight_out != NULL)
            *tot_weight_out = KdArc::Weight::Zero();
        return true;
    }
    while (1)
    {
        KdArc::Weight w = fst.Final(cur_state);
        if (w != KdArc::Weight::Zero())
        {  // is final..
            tot_weight = Times(w, tot_weight);
            if (fst.NumArcs(cur_state) != 0)
                return false;
            if (isymbols_out != NULL)
                *isymbols_out = ilabel_seq;
            if (osymbols_out != NULL)
                *osymbols_out = olabel_seq;
            if (tot_weight_out != NULL)
                *tot_weight_out = tot_weight;
            return true;
        }
        else
        {
            if (fst.NumArcs(cur_state) != 1)
                return false;

            fst::ArcIterator<fst::Fst<KdArc> > iter(fst, cur_state);  // get the only arc.
            const KdArc &arc = iter.Value();
            tot_weight = Times(arc.weight, tot_weight);
            if (arc.ilabel != 0)
                ilabel_seq.push_back(arc.ilabel);
            if (arc.olabel != 0)
                olabel_seq.push_back(arc.olabel);
            cur_state = arc.nextstate;
        }
    }
}


KdStateId CreateSuperFinal(KdCompactLattice *fst)
{
    assert(fst != NULL);

    KdStateId num_states = fst->NumStates();
    KdStateId num_final = 0;

    std::vector<KdStateId> final_states;
    for (KdStateId s = 0; s < num_states; s++)
    {
        if (fst->Final(s) != KdCLatWeight::Zero())
        {
            num_final++;
            final_states.push_back(s);
        }
    }
    if (final_states.size() == 1)
    {
        if (fst->Final(final_states[0]) == KdCLatWeight::One())
        {
            fst::ArcIterator<KdCompactLattice> iter(*fst, final_states[0]);
            if (iter.Done())
            {
                // We already have a final state w/ no transitions out and unit weight.
                // So we're done.
                return final_states[0];
            }
        }
    }

    KdStateId final_state = fst->AddState();
    fst->SetFinal(final_state, KdCLatWeight::One());
    for (size_t idx = 0; idx < final_states.size(); idx++)
    {
        KdStateId s = final_states[idx];
        KdCLatWeight weight = fst->Final(s);
        fst->SetFinal(s, KdCLatWeight::Zero());
        KdCLatArc arc;
        arc.ilabel = 0;
        arc.olabel = 0;
        arc.nextstate = final_state;
        arc.weight = weight;
        fst->AddArc(s, arc);
    }
    return final_state;
}

#endif // KD_FST_UTIL_H
