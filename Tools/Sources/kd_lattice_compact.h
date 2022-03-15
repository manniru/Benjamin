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
                    KdCompactLattice *ofst, bool invert = true);


void ConvertLattice(const KdCompactLattice &ifst,
        KdLattice *ofst, bool invert = true)
{
    ofst->DeleteStates();
    // make the states in the new FST have the same numbers as
    // the original ones, and add chains of states as necessary
    // to encode the string-valued weights.
    KdStateId num_states = ifst.NumStates();
    for (KdStateId s = 0; s < num_states; s++)
    {
        KdStateId news = ofst->AddState();
        assert(news == s);
    }
    ofst->SetStart(ifst.Start());
    for (KdStateId s = 0; s < num_states; s++)
    {
        KdCLatWeight final_weight = ifst.Final(s);
        if (final_weight != KdCLatWeight::Zero())
        {
            KdStateId cur_state = s;
            size_t string_length = final_weight.String().size();
            for (size_t n = 0; n < string_length; n++) {
                KdStateId next_state = ofst->AddState();
                int ilabel = 0;
                KdLatticeArc arc(ilabel, final_weight.String()[n],
                        (n == 0 ? final_weight.Weight() : KdLatticeWeight::One()),
                        next_state);
                if (invert) std::swap(arc.ilabel, arc.olabel);
                ofst->AddArc(cur_state, arc);
                cur_state = next_state;
            }
            ofst->SetFinal(cur_state,
                           string_length > 0 ? KdLatticeWeight::One() : final_weight.Weight());
        }
        for( fst::ArcIterator<fst::ExpandedFst<KdCLatArc> > iter(ifst, s);
             !iter.Done() ; iter.Next())
        {
            const KdCLatArc &arc = iter.Value();
            size_t string_length = arc.weight.String().size();
            KdStateId cur_state = s;
            // for all but the last element in the string--
            // add a temporary state.
            for( size_t n=0 ; n+1<string_length ; n++ )
            {
                KdStateId next_state = ofst->AddState();
                int ilabel = (n == 0 ? arc.ilabel : 0);
                int olabel = static_cast<int>(arc.weight.String()[n]);
                KdLatticeWeight weight = (n == 0 ? arc.weight.Weight() : KdLatticeWeight::One());
                KdLatticeArc new_arc(ilabel, olabel, weight, next_state);
                if( invert )
                {
                    std::swap(new_arc.ilabel, new_arc.olabel);
                }
                ofst->AddArc(cur_state, new_arc);
                cur_state = next_state;
            }
            int ilabel = (string_length <= 1 ? arc.ilabel : 0);
            int olabel = (string_length > 0 ? arc.weight.String()[string_length-1] : 0);
            KdLatticeWeight weight = (string_length <= 1 ? arc.weight.Weight() : KdLatticeWeight::One());
            KdLatticeArc new_arc(ilabel, olabel, weight, arc.nextstate);
            if( invert )
            {
                std::swap(new_arc.ilabel, new_arc.olabel);
            }
            ofst->AddArc(cur_state, new_arc);
        }
    }
}



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
