#ifndef KD_FASTER_TOKEN_H
#define KD_FASTER_TOKEN_H

#include "fst/arc.h"

class KdFToken
{
public:
    KdFToken(fst::StdArc &arc, float ac_cost, KdFToken *prev);
    KdFToken(fst::StdArc &arc, KdFToken *prev);

    bool operator < (KdFToken &other);

    fst::StdArc arc_;
    KdFToken *prev_;
    int       ref_count; //how many token refrenced
    double cost;
};

void KdFTokenDelete(KdFToken *tok);

#endif // KD_FASTER_TOKEN_H
