#include "kd_fst_util.h"

bool kd_GetLinearSymbolSequence(const fst::Fst<KdArc> &fst, std::vector<int> *isymbols_out, std::vector<int> *osymbols_out, KdArc::Weight *tot_weight_out)
{
    KdArc::Weight tot_weight = KdArc::Weight::One();
    std::vector<int> ilabel_seq;
    std::vector<int> olabel_seq;

    KdStateId cur_state = fst.Start();
    if (cur_state==fst::kNoStateId)
    {  // empty sequence.
        if (isymbols_out!=NULL)
            isymbols_out->clear();
        if (osymbols_out!=NULL)
            osymbols_out->clear();
        if (tot_weight_out!=NULL)
            *tot_weight_out = KdArc::Weight::Zero();
        return true;
    }
    while (1)
    {
        KdArc::Weight w = fst.Final(cur_state);
        if (w!=KdArc::Weight::Zero())
        {  // is final..
            tot_weight = Times(w, tot_weight);
            if (fst.NumArcs(cur_state)!=0)
                return false;
            if (isymbols_out!=NULL)
                *isymbols_out = ilabel_seq;
            if (osymbols_out!=NULL)
                *osymbols_out = olabel_seq;
            if (tot_weight_out!=NULL)
                *tot_weight_out = tot_weight;
            return true;
        }
        else
        {
            if (fst.NumArcs(cur_state)!=1)
                return false;

            fst::ArcIterator<fst::Fst<KdArc> > iter(fst, cur_state);  // get the only arc.
            const KdArc &arc = iter.Value();
            tot_weight = Times(arc.weight, tot_weight);
            if (arc.ilabel!=0)
                ilabel_seq.push_back(arc.ilabel);
            if (arc.olabel!=0)
                olabel_seq.push_back(arc.olabel);
            cur_state = arc.nextstate;
        }
    }
}
