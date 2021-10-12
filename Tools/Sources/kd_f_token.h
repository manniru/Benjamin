#ifndef KD_FASTER_TOKEN_H
#define KD_FASTER_TOKEN_H

#include "decoder/faster-decoder.h"


class KdFToken
{
public:
    KdFToken::KdFToken(fst::StdArc &arc, float ac_cost, KdFToken *prev);
    KdFToken::KdFToken(fst::StdArc &arc, KdFToken *prev);

    bool KdFToken::operator < (KdFToken &other);

    void KdFToken::KdFTokenDelete(KdFToken *tok);

    fst::StdArc arc_; // contains only the graph part of the cost;
    // we can work out the acoustic part from difference between
    // "cost_" and prev->cost_.
    KdFToken *prev_;
    int       ref_count_;
    // if you are looking for weight_ here, it was removed and now we just have
    // cost_, which corresponds to ConvertToCost(weight_).
    double cost_;
};

#endif // KD_FASTER_TOKEN_H
