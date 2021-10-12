#ifndef KD_FASTER_TOKEN_H
#define KD_FASTER_TOKEN_H

#include "decoder/faster-decoder.h"


class KdFToken
{
public:
    KdFToken(fst::StdArc &arc, float ac_cost, KdFToken *prev);
    KdFToken(fst::StdArc &arc, KdFToken *prev);

    bool operator < (KdFToken &other);

    fst::StdArc arc_; // contains only the graph part of the cost;
    // we can work out the acoustic part from difference between
    // "cost_" and prev->cost_.
    KdFToken *prev_;
    int       ref_count_;
    double cost_;
};

void KdFTokenDelete(KdFToken *tok);

#endif // KD_FASTER_TOKEN_H
