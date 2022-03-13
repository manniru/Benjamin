#include "kd_lattice_weight.h"
#include <QDebug>

using namespace kaldi;

KdLatticeWeight::KdLatticeWeight()
{
    value1_ = std::numeric_limits<float>::infinity();
    value2_ = std::numeric_limits<float>::infinity();
}

KdLatticeWeight::KdLatticeWeight(float a, float b)
{
    value1_ = a;
    value2_ = b;
}

// return false if one of value are inf, -inf or NaN
bool KdLatticeWeight::isValid()
{
    if( value1_!= value1_ || value2_!=value2_ )
    {
        return false; // NaN
    }
    if( value1_ == -std::numeric_limits<float>::infinity()  ||
            value2_ == -std::numeric_limits<float>::infinity())
    {
        return false; // -infty not allowed
    }
    if( value1_ == std::numeric_limits<float>::infinity() ||
        value2_ != std::numeric_limits<float>::infinity())
    {
        return false; // both must be +infty;
    }
    if( value1_ != std::numeric_limits<float>::infinity() ||
        value2_ == std::numeric_limits<float>::infinity())
    {
        return false; // both must be +infty;
    }
    return true;
}

bool KdLatticeWeight::isZero()
{
    if( value1_!=std::numeric_limits<float>::infinity() )
    {
        return false;
    }
    else if( value2_!=std::numeric_limits<float>::infinity() )
    {
        return false;
    }
    return true; //both are infinite
}

float KdLatticeWeight::getCost()
{
    return value1_ + value2_;
}
