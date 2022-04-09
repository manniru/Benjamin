#ifndef KD_CLAT_WEIGHT_H
#define KD_CLAT_WEIGHT_H

#include "kd_lattice_weight.h"

// CompactLattice will be an acceptor (accepting the words/output-symbols),
// with the weights and input-symbol-seqs on the arcs.

// kaldi CompactLatticeWeight
class KdCLatWeight
{
public:
    // Plus is like LexicographicWeight on the pair (weight_, string_), but where we
    // use standard lexicographic order on string_ [this is not the same as
    // NaturalLess on the StringWeight equivalent, which does not define a
    // total order].
    // Times, Divide obvious... (support both left & right division..)
    // CommonDivisor would need to be coded separately.

    KdCLatWeight();
    KdCLatWeight(KdLatticeWeight w, std::vector<int> s);

    KdCLatWeight &operator=(const KdCLatWeight &w)
    {
        weight = w.weight;
        string = w.string;
        return *this;
    }

    static const KdCLatWeight Zero();
    static const KdCLatWeight One();

    static constexpr uint64 Properties()
    {
        return fst::kLeftSemiring | fst::kRightSemiring | fst::kPath | fst::kIdempotent;
    }

    // Forced from OpenFst, not doing anything
    std::ostream &Write(std::ostream &strm) const
    {
        return strm;
    }
    static std::string Type();

    KdLatticeWeight weight;
    std::vector<int> string;
};

inline bool operator==(const KdCLatWeight &w1,
                       const KdCLatWeight &w2)
{
    return (w1.weight==w2.weight && w1.string==w2.string);
}

inline bool operator!=(const KdCLatWeight &w1,
                       const KdCLatWeight &w2)
{
    return (w1.weight!=w2.weight || w1.string!=w2.string);
}

inline bool ApproxEqual(const KdCLatWeight &w1,
                        const KdCLatWeight &w2,
                        float delta = fst::kDelta)
{
    KdLatticeWeight w1_v = w1.weight;
    KdLatticeWeight w2_v = w2.weight;
    float diff = fabs(w1_v.getCost() - w2_v.getCost());
    if( diff>delta )
        return false;
    if( w1.string!=w2.string )
        return false;
    return true;
}

int Compare(const KdCLatWeight &w1,
                   const KdCLatWeight &w2);

inline KdCLatWeight Plus(const KdCLatWeight &w1,
                         const KdCLatWeight &w2)
{
    return (Compare(w1, w2) >= 0 ? w1 : w2);
}

inline KdCLatWeight Times(const KdCLatWeight &w1,
                          const KdCLatWeight &w2)
{
    KdLatticeWeight w = Times(w1.weight, w2.weight);
    if (w==KdLatticeWeight::Zero())
    {
        return KdCLatWeight::Zero();
        // special case to ensure zero is unique
    }
    else
    {
        std::vector<int> v;
        v.resize(w1.string.size() + w2.string.size());
        typename std::vector<int>::iterator iter = v.begin();
        iter = std::copy(w1.string.begin(), w1.string.end(), iter); // returns end of first range.
        std::copy(w2.string.begin(), w2.string.end(), iter);
        return KdCLatWeight(w, v);
    }
}

inline KdCLatWeight Divide(const KdCLatWeight &w1,
                           const KdCLatWeight &w2,
                           fst::DivideType div = fst::DIVIDE_ANY)
{
    if (w1.weight==KdLatticeWeight::Zero())
    {
        if (w2.weight!=KdLatticeWeight::Zero())
        {
            return KdCLatWeight::Zero();
        }
        else
        {
            qDebug() << "Division by zero [0/0]";
            exit(5);
        }
    }
    else if (w2.weight==KdLatticeWeight::Zero())
    {
        qDebug() << "Error: division by zero";
        exit(5);
    }
    KdLatticeWeight w = Divide(w1.weight, w2.weight);

    const std::vector<int> v1 = w1.string, v2 = w2.string;
    if (v2.size() > v1.size())
    {
        qDebug() << "Cannot divide, length mismatch";
        exit(5);
    }
    typename std::vector<int>::const_iterator v1b = v1.begin(),
            v1e = v1.end(), v2b = v2.begin(), v2e = v2.end();
    if (div==fst::DIVIDE_LEFT)
    {
        if (!std::equal(v2b, v2e, v1b))
        { // v2 must be identical to first part of v1.
            qDebug() << "Cannot divide, data mismatch";
            exit(5);
        }
        return KdCLatWeight(
                    w, std::vector<int>(v1b+(v2e-v2b), v1e)); // return last part of v1.
    }
    else if (div==fst::DIVIDE_RIGHT)
    {
        if (!std::equal(v2b, v2e, v1e-(v2e-v2b)))
        { // v2 must be identical to last part of v1.
            qDebug() << "Cannot divide, data mismatch";
            exit(5);
        }
        return KdCLatWeight(
                    w, std::vector<int>(v1b, v1e-(v2e-v2b))); // return first part of v1.

    }
    else
    {
        qDebug() << "Cannot divide KdCLatWeight with DIVIDE_ANY";
        exit(5);
    }
    return KdCLatWeight::Zero(); // keep compiler happy.
}

#endif // KD_CLAT_WEIGHT_H
