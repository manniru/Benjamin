#ifndef KD_CLAT_WEIGHT_H
#define KD_CLAT_WEIGHT_H

#include "kd_lattice_weight.h"

// CompactLattice will be an acceptor (accepting the words/output-symbols),
// with the weights and input-symbol-seqs on the arcs.
// There must be a total order on KdLatticeWeight.  We assume for the sake of efficiency
// that there is a function
// Compare(KdLatticeWeight w1, KdLatticeWeight w2) that returns -1 if w1 < w2, +1 if w1 > w2, and
// zero if w1 == w2, and Plus for type KdLatticeWeight returns (Compare(w1,w2) >= 0 ? w1 : w2).

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

    KdCLatWeight() { }

    KdCLatWeight(const KdLatticeWeight &w, const std::vector<int> &s):
        weight_(w), string_(s) { }

    KdCLatWeight &operator=(const KdCLatWeight &w)
    {
        weight_ = w.weight_;
        string_ = w.string_;
        return *this;
    }

    const KdLatticeWeight &Weight() const { return weight_; }

    const std::vector<int> &String() const { return string_; }

    void SetWeight(const KdLatticeWeight &w) { weight_ = w; }

    void SetString(const std::vector<int> &s) { string_ = s; }

    static const KdCLatWeight Zero()
    {
        return KdCLatWeight(KdLatticeWeight::Zero(), std::vector<int>());
    }

    static const KdCLatWeight One()
    {
        return KdCLatWeight(
                    KdLatticeWeight::One(), std::vector<int>());
    }

    inline static std::string GetintSizeString()
    {
        char buf[2];
        buf[0] = '0' + sizeof(int);
        buf[1] = '\0';
        return buf;
    }
    static const std::string &Type()
    {
        static const std::string type = "compact" + KdLatticeWeight::Type()
                + GetintSizeString();
        return type;
    }

    static const KdCLatWeight NoWeight()
    {
        return KdCLatWeight(
                    KdLatticeWeight::NoWeight(), std::vector<int>());
    }


    KdCLatWeight Reverse() const
    {
        size_t s = string_.size();
        std::vector<int> v(s);
        for(size_t i = 0; i < s; i++)
            v[i] = string_[s-i-1];
        return KdCLatWeight(weight_, v);
    }

    bool Member()
    {
        // a semiring has only one zero, this is the important property
        // we're trying to maintain here.  So force string_ to be empty if
        // w_ == zero.
        if( !weight_.isValid() )
            return false;
        if (weight_ == KdLatticeWeight::Zero())
            return string_.empty();
        else
            return true;
    }

    static constexpr uint64 Properties()
    {
        return fst::kLeftSemiring | fst::kRightSemiring | fst::kPath | fst::kIdempotent;
    }

    // This is used in OpenFst for binary I/O.  This is OpenFst-style,
    // not Kaldi-style, I/O.
    std::ostream &Write(std::ostream &strm) const
    {
        weight_.Write(strm);
        if (strm.fail())
        {
            return strm;
        }
        int32 sz = string_.size();
        fst::WriteType(strm, sz);
        for(int32 i = 0; i < sz; i++)
        {
            fst::WriteType(strm, string_[i]);
        }
        return strm;
    }

private:
    KdLatticeWeight weight_;
    std::vector<int> string_;

};

inline bool operator==(const KdCLatWeight &w1,
                       const KdCLatWeight &w2)
{
    return (w1.Weight() == w2.Weight() && w1.String() == w2.String());
}

inline bool operator!=(const KdCLatWeight &w1,
                       const KdCLatWeight &w2)
{
    return (w1.Weight() != w2.Weight() || w1.String() != w2.String());
}

inline bool ApproxEqual(const KdCLatWeight &w1,
                        const KdCLatWeight &w2,
                        float delta = fst::kDelta)
{
    KdLatticeWeight w1_v = w1.Weight();
    KdLatticeWeight w2_v = w2.Weight();
    float diff = fabs(w1_v.getCost() - w2_v.getCost());
    if( diff>delta )
        return false;
    if( w1.String()!=w2.String() )
        return false;
    return true;
}



// Compare is not part of the standard for weight types, but used internally for
// efficiency.  The comparison here first compares the weight; if this is the
// same, it compares the string.  The comparison on strings is: first compare
// the length, if this is the same, use lexicographical order.  We can't just
// use the lexicographical order because this would destroy the distributive
// property of multiplication over addition, taking into account that addition
// uses Compare.  The string element of "Compare" isn't super-important in
// practical terms; it's only needed to ensure that Plus always give consistent
// answers and is symmetric.  It's essentially for tie-breaking, but we need to
// make sure all the semiring axioms are satisfied otherwise OpenFst might
// break.

inline int Compare(const KdCLatWeight &w1,
                   const KdCLatWeight &w2)
{
    int c1 = Compare(w1.Weight(), w2.Weight());
    if (c1 != 0) return c1;
    int l1 = w1.String().size(), l2 = w2.String().size();
    // Use opposite order on the string lengths, so that if the costs are the same,
    // the shorter string wins.
    if (l1 > l2) return -1;
    else if (l1 < l2) return 1;
    for(int i = 0; i < l1; i++) {
        if (w1.String()[i] < w2.String()[i]) return -1;
        else if (w1.String()[i] > w2.String()[i]) return 1;
    }
    return 0;
}

inline KdCLatWeight Plus(
        const KdCLatWeight &w1,
        const KdCLatWeight &w2) {
    return (Compare(w1, w2) >= 0 ? w1 : w2);
}

inline KdCLatWeight Times(
        const KdCLatWeight &w1,
        const KdCLatWeight &w2)
{
    KdLatticeWeight w = Times(w1.Weight(), w2.Weight());
    if (w == KdLatticeWeight::Zero())
    {
        return KdCLatWeight::Zero();
        // special case to ensure zero is unique
    }
    else
    {
        std::vector<int> v;
        v.resize(w1.String().size() + w2.String().size());
        typename std::vector<int>::iterator iter = v.begin();
        iter = std::copy(w1.String().begin(), w1.String().end(), iter); // returns end of first range.
        std::copy(w2.String().begin(), w2.String().end(), iter);
        return KdCLatWeight(w, v);
    }
}

inline KdCLatWeight Divide(const KdCLatWeight &w1,
                           const KdCLatWeight &w2,
                           fst::DivideType div = fst::DIVIDE_ANY)
{
    if (w1.Weight() == KdLatticeWeight::Zero())
    {
        if (w2.Weight() != KdLatticeWeight::Zero())
        {
            return KdCLatWeight::Zero();
        }
        else
        {
            KALDI_ERR << "Division by zero [0/0]";
        }
    }
    else if (w2.Weight() == KdLatticeWeight::Zero())
    {
        KALDI_ERR << "Error: division by zero";
    }
    KdLatticeWeight w = Divide(w1.Weight(), w2.Weight());

    const std::vector<int> v1 = w1.String(), v2 = w2.String();
    if (v2.size() > v1.size())
    {
        KALDI_ERR << "Cannot divide, length mismatch";
    }
    typename std::vector<int>::const_iterator v1b = v1.begin(),
            v1e = v1.end(), v2b = v2.begin(), v2e = v2.end();
    if (div == fst::DIVIDE_LEFT)
    {
        if (!std::equal(v2b, v2e, v1b))
        { // v2 must be identical to first part of v1.
            KALDI_ERR << "Cannot divide, data mismatch";
        }
        return KdCLatWeight(
                    w, std::vector<int>(v1b+(v2e-v2b), v1e)); // return last part of v1.
    }
    else if (div == fst::DIVIDE_RIGHT)
    {
        if (!std::equal(v2b, v2e, v1e-(v2e-v2b))) { // v2 must be identical to last part of v1.
            KALDI_ERR << "Cannot divide, data mismatch";
        }
        return KdCLatWeight(
                    w, std::vector<int>(v1b, v1e-(v2e-v2b))); // return first part of v1.

    }
    else
    {
        KALDI_ERR << "Cannot divide KdCLatWeight with DIVIDE_ANY";
    }
    return KdCLatWeight::Zero(); // keep compiler happy.
}



inline void ConvertLatticeWeight(const KdCLatWeight &w_in,
        KdCLatWeight *w_out)
{
    KdLatticeWeight weight2(w_in.Weight().Value1(),
                            w_in.Weight().Value2());
    w_out->SetWeight(weight2);
    w_out->SetString(w_in.String());
}
#endif // KD_CLAT_WEIGHT_H
