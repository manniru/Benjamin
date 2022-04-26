#include "kd_lattice_weight.h"
#include <QDebug>

KdLatticeWeight::KdLatticeWeight()
{
    g_cost = KD_INFINITY_FL;
    a_cost = KD_INFINITY_FL;
}

KdLatticeWeight::KdLatticeWeight(float gcost, float acost)
{
    g_cost = gcost;
    a_cost = acost;
}

// return false if one of value are inf, -inf or NaN
bool KdLatticeWeight::isValid()
{
    if( g_cost!= g_cost || a_cost!=a_cost )
    {
        return false; // NaN
    }
    if( g_cost==-KD_INFINITY_FL  ||
            a_cost==-KD_INFINITY_FL)
    {
        return false; // -infty not allowed
    }
    if( g_cost==KD_INFINITY_FL ||
        a_cost!=KD_INFINITY_FL)
    {
        return false; // both must be +infty;
    }
    if( g_cost!=KD_INFINITY_FL ||
        a_cost==KD_INFINITY_FL)
    {
        return false; // both must be +infty;
    }
    return true;
}

bool KdLatticeWeight::isZero()
{
    if( g_cost!=KD_INFINITY_FL )
    {
        return false;
    }
    else if( a_cost!=KD_INFINITY_FL )
    {
        return false;
    }
    return true; //both are infinite
}

float KdLatticeWeight::getCost()
{
    return g_cost + a_cost;
}

void ConvertLatticeWeight(const KdLatticeWeight &w_in, fst::TropicalWeightTpl<float> *w_out)
{
    fst::TropicalWeightTpl<float> w1(w_in.g_cost);
    fst::TropicalWeightTpl<float> w2(w_in.a_cost);
    *w_out = Times(w1, w2);
}


