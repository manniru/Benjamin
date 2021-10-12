#include "kd_f_token.h"
#include <QDebug>

using namespace kaldi;

KdFToken::KdFToken(fst::StdArc &arc, float ac_cost, KdFToken *prev):
    arc_(arc), prev_(prev), ref_count_(1)
{
    if (prev)
    {
        prev->ref_count_++;
        cost_ = prev->cost_ + arc.weight.Value() + ac_cost;
    }
    else
    {
        cost_ = arc.weight.Value() + ac_cost;
    }
}

KdFToken::KdFToken(fst::StdArc &arc, KdFToken *prev):
    arc_(arc), prev_(prev), ref_count_(1)
{
    if (prev)
    {
        prev->ref_count_++;
        cost_ = prev->cost_ + arc.weight.Value();
    } else
    {
        cost_ = arc.weight.Value();
    }
}

bool KdFToken::operator < (KdFToken &other)
{
    return cost_ > other.cost_;
}

void KdFToken::KdFTokenDelete(KdFToken *tok)
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
        else tok = prev;
    }
}
