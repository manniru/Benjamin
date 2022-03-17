#include "kd_lattice_weight.h"
#include <QDebug>

KdLatticeWeight::KdLatticeWeight()
{
    value1 = std::numeric_limits<float>::infinity();
    value2 = std::numeric_limits<float>::infinity();
}

KdLatticeWeight::KdLatticeWeight(float a, float b)
{
    value1 = a;
    value2 = b;
}

// return false if one of value are inf, -inf or NaN
bool KdLatticeWeight::isValid()
{
    if( value1!= value1 || value2!=value2 )
    {
        return false; // NaN
    }
    if( value1 == -std::numeric_limits<float>::infinity()  ||
            value2 == -std::numeric_limits<float>::infinity())
    {
        return false; // -infty not allowed
    }
    if( value1 == std::numeric_limits<float>::infinity() ||
        value2 != std::numeric_limits<float>::infinity())
    {
        return false; // both must be +infty;
    }
    if( value1 != std::numeric_limits<float>::infinity() ||
        value2 == std::numeric_limits<float>::infinity())
    {
        return false; // both must be +infty;
    }
    return true;
}

bool KdLatticeWeight::isZero()
{
    if( value1!=std::numeric_limits<float>::infinity() )
    {
        return false;
    }
    else if( value2!=std::numeric_limits<float>::infinity() )
    {
        return false;
    }
    return true; //both are infinite
}

float KdLatticeWeight::getCost()
{
    return value1 + value2;
}

void ConvertLatticeWeight(const KdLatticeWeight &w_in, fst::TropicalWeightTpl<float> *w_out)
{
    fst::TropicalWeightTpl<float> w1(w_in.value1);
    fst::TropicalWeightTpl<float> w2(w_in.value2);
    *w_out = Times(w1, w2);
}


