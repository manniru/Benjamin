#include "kd_f_token.h"
#include <QDebug>

using namespace kaldi;

KdFToken::KdFToken(fst::StdArc &arc, float ac_cost, KdFToken *prev):
    arc_(arc), prev_(prev), ref_count_(1)
{
    cost_ = arc.weight.Value() + ac_cost;

    if( prev )
    {
        prev->ref_count_++;
        cost_ += prev->cost_;
    }
}

KdFToken::KdFToken(fst::StdArc &arc, KdFToken *prev):
    arc_(arc), prev_(prev), ref_count_(1)
{
    cost_ = arc.weight.Value();

    if( prev )
    {
        prev->ref_count_++;
        cost_ += prev->cost_;
    }
}

bool KdFToken::operator < (KdFToken &other)
{
    return other.cost_<cost_;
}

void KdFTokenDelete(KdFToken *tok)
{
    while( tok->ref_count_==0 )
    {
        tok--;
        KdFToken *prev = tok->prev_;
        delete tok;

        if (prev == NULL)
        {
            return;
        }
        else
        {
            tok = prev;
        }
    }
}
