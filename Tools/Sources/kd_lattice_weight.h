#ifndef KD_LATTICE_WEIGHT_H
#define KD_LATTICE_WEIGHT_H

#include <QDebug>
#include "bt_config.h"
#include <fstext/fstext-lib.h>
#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <util/kaldi-table.h>

class KdLatticeWeight
{
public:
    inline float Value1() const { return value1_; }

    inline float Value2() const { return value2_; }

    inline void SetValue1(float f) { value1_ = f; }

    inline void SetValue2(float f) { value2_ = f; }

    KdLatticeWeight();
    KdLatticeWeight(float a, float b);

    KdLatticeWeight &operator=(const KdLatticeWeight &w) {
        value1_ = w.value1_;
        value2_ = w.value2_;
        return *this;
    }

    KdLatticeWeight Reverse() const
    {
        return *this;
    }

    static const KdLatticeWeight Zero()
    {
        return KdLatticeWeight(std::numeric_limits<float>::infinity(),
                               std::numeric_limits<float>::infinity());
    }

    static const KdLatticeWeight One()
    {
        return KdLatticeWeight(0.0, 0.0);
    }

    static const std::string &Type()
    {
        static const std::string type = (sizeof(float) == 4 ? "lattice4" : "lattice8") ;
        return type;
    }

    static const KdLatticeWeight NoWeight()
    {
        return KdLatticeWeight(std::numeric_limits<float>::quiet_NaN(),
                               std::numeric_limits<float>::quiet_NaN());
    }

    bool isValid();
    bool isZero();
    float getCost();

    static constexpr uint64 Properties()
    {
        return fst::kLeftSemiring | fst::kRightSemiring | fst::kCommutative |
                fst::kPath | fst::kIdempotent;
    }

    ///FIXME: SHOULD BE REMOVED
    std::ostream &Write(std::ostream &strm) const
    {
        return strm;
    }

private:
    float value1_;
    float value2_;
};

inline bool operator!=(const KdLatticeWeight &wa,
                       const KdLatticeWeight &wb)
{
    // Volatile qualifier thwarts over-aggressive compiler optimizations
    // that lead to problems esp. with NaturalLess().
    float va1 = wa.Value1();
    float va2 = wa.Value2();
    float vb1 = wb.Value1();
    float vb2 = wb.Value2();
    return (va1 != vb1 || va2 != vb2);
}

inline KdLatticeWeight Times(const KdLatticeWeight &w1,
                             const KdLatticeWeight &w2)
{
    return KdLatticeWeight(w1.Value1()+w2.Value1(), w1.Value2()+w2.Value2());
}

inline int Compare (const KdLatticeWeight &w1,
                    const KdLatticeWeight &w2)
{
    float f1 = w1.Value1() + w1.Value2();
    float f2 = w2.Value1() + w2.Value2();
    if( f1 < f2)
    {
        return 1;
    } // having smaller cost means you're larger
    // in the semiring [higher probability]
    else if( f1 > f2)
    {
        return -1;
    }
    // mathematically we should be comparing (w1.value1_-w1.value2_ < w2.value1_-w2.value2_)
    // in the next line, but add w1.value1_+w1.value2_ = w2.value1_+w2.value2_ to both sides and
    // divide by two, and we get the simpler equivalent form w1.value1_ < w2.value1_.
    else if( w1.Value1() < w2.Value1())
    {
        return 1;
    }
    else if( w1.Value1() > w2.Value1())
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

inline KdLatticeWeight Plus(const KdLatticeWeight &w1,
                            const KdLatticeWeight &w2)
{
    return (Compare(w1, w2) >= 0 ? w1 : w2);
}

// divide w1 by w2 (on left/right/any doesn't matter as
// commutative).
inline KdLatticeWeight Divide(const KdLatticeWeight &w1,
                              const KdLatticeWeight &w2,
                              fst::DivideType=fst::DIVIDE_LEFT)
{
    float a = w1.Value1() - w2.Value1();
    float b = w1.Value2() - w2.Value2();

    if( a != a || b != b || a == -std::numeric_limits<float>::infinity()
            || b == -std::numeric_limits<float>::infinity())
    {
        KALDI_WARN << "LatticeWeightTpl::Divide, NaN or invalid number produced. "
                   << "[dividing by zero?]  Returning zero";
        return KdLatticeWeight::Zero();
    }
    if( a == std::numeric_limits<float>::infinity() ||
            b == std::numeric_limits<float>::infinity())
    {
        return KdLatticeWeight::Zero(); // not a valid number if only one is infinite.
    }
    return KdLatticeWeight(a, b);
}
inline bool operator==(const KdLatticeWeight &wa,
                       const KdLatticeWeight &wb)
{
    // Volatile qualifier thwarts over-aggressive compiler optimizations
    // that lead to problems esp. with NaturalLess().
    volatile float va1 = wa.Value1(), va2 = wa.Value2(),
            vb1 = wb.Value1(), vb2 = wb.Value2();
    return (va1 == vb1 && va2 == vb2);
}

// to convert from Lattice to standard FST
inline void ConvertLatticeWeight(const KdLatticeWeight &w_in,
                                 fst::TropicalWeightTpl<float> *w_out)
{
    fst::TropicalWeightTpl<float> w1(w_in.Value1());
    fst::TropicalWeightTpl<float> w2(w_in.Value2());
    *w_out = Times(w1, w2);
}

#endif // KD_LATTICE_WEIGHT_H
