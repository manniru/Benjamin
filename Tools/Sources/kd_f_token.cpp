#include "kd_f_token.h"
#include <QDebug>

using namespace kaldi;

KdFToken::KdFToken(fst::StdArc &arc, float ac_cost, KdFToken *prev):
    arc_(arc), prev_(prev)
{
    cost = arc.weight.Value() + ac_cost;
    ref_count = 1;

    if( prev )
    {
        prev->ref_count++;
        cost += prev->cost;
    }
}

KdFToken::KdFToken(fst::StdArc &arc, KdFToken *prev):
    arc_(arc), prev_(prev)
{
    cost = arc.weight.Value();
    ref_count = 1;

    if( prev )
    {
        prev->ref_count++;
        cost += prev->cost;
    }
}

bool KdFToken::operator < (KdFToken &other)
{
    return other.cost<cost;
}

void KdFTokenDelete(KdFToken *tok)
{
    while( tok->ref_count==0 )
    {
        tok--;
        KdFToken *prev = tok->prev_;
        delete tok;

        if( prev==NULL)
        {
            return;
        }
        else
        {
            tok = prev;
        }
    }
}
