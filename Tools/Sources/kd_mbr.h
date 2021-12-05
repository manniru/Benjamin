#ifndef KD_MBR_H
#define KD_MBR_H

// described in "Minimum Bayes Risk decoding and system combination based on a recursion for
// edit distance", Haihua Xu, Daniel Povey, Lidia Mangu and Jie Zhu,

#include "lat/sausages.h"
#include "lat/lattice-functions.h"

class KdMBR
{
public:
    KdMBR(kaldi::CompactLattice *clat);

    std::vector<int32> GetOneBest();
    std::vector<std::vector<std::pair<float, float> > > GetTimes();
    std::vector<std::pair<float, float> > GetSausageTimes();
    std::vector<std::pair<float, float> > GetOneBestTimes();
    std::vector<float> GetOneBestConfidences();

private:
    void PrepareLatticeAndInitStats(kaldi::CompactLattice *clat);

    void MbrDecode(); // Figure 6 of the paper.

    /// Without the 'penalize' argument this gives us the basic edit-distance
    /// function l(a,b), as in the paper.
    /// With the 'penalize' argument it can be interpreted as the edit distance
    /// plus the 'delta' from the paper, except that we make a kind of conceptual
    /// bug-fix and only apply the delta if the edit-distance was not already
    /// zero.  This bug-fix was necessary in order to force all the stats to show
    /// up, that should show up, and applying the bug-fix makes the sausage stats
    /// significantly less sparse.
    inline double l(int32 a, int32 b, bool penalize = false)
    {
        if (a == b) return 0.0;
        else return (penalize ? 1.0 + delta() : 1.0);
    }

    /// returns r_q, in one-based indexing, as in the paper.
    inline int32 r(int32 q) { return R_[q-1]; }


    /// Figure 4 of the paper; called from AccStats (Fig. 5)
    double EditDistance(int32 N, int32 Q,
                        kaldi::Vector<double> &alpha,
                        kaldi::Matrix<double> &alpha_dash,
                        kaldi::Vector<double> &alpha_dash_arc);

    /// Figure 5 of the paper.  Outputs to gamma_ and L_.
    void AccStats();

    /// Removes epsilons (symbol 0) from a vector
    static void RemoveEps(std::vector<int32> *vec);

    // Ensures that between each word in "vec" and at the beginning and end, is
    // epsilon (0).  (But if no words in vec, just one epsilon)
    static void NormalizeEps(std::vector<int32> *vec);

    // delta() is a constant used in the algorithm, which penalizes
    // the use of certain epsilon transitions in the edit-distance which would cause
    // words not to show up in the accumulated edit-distance statistics.
    // There has been a conceptual bug-fix versus the way it was presented in
    // the paper: we now add delta only if the edit-distance was not already
    // zero.
    static inline float delta() { return 1.0e-05; }


    /// Function used to increment map.
    static inline void AddToMap(int32 i, double d, std::map<int32, double> *gamma)
    {
        if (d == 0) return;
        std::pair<const int32, double> pr(i, d);
        std::pair<std::map<int32, double>::iterator, bool> ret = gamma->insert(pr);
        if (!ret.second) // not inserted, so add to contents.
            ret.first->second += d;
    }

    struct Arc
    {
        int32 word;
        int32 start_node;
        int32 end_node;
        float loglike;
    };

    kaldi::MinimumBayesRiskOptions opts;


    /// Arcs in the topologically sorted acceptor form of the word-level lattice,
    /// with one final-state.  Contains (word-symbol, log-likelihood on arc ==
    /// negated cost).  Indexed from zero.
    std::vector<Arc> arcs_;

    /// For each node in the lattice, a list of arcs entering that node. Indexed
    /// from 1 (first node == 1).
    std::vector<std::vector<int32> > pre_;

    std::vector<int32> state_times_; // time of each state in the word lattice,
    // indexed from 1 (same index as into pre_)

    std::vector<int32> R_; // current 1-best word sequence, normalized to have
    // epsilons between each word and at the beginning and end.  R in paper...
    // caution: indexed from zero, not from 1 as in paper.

    double L_; // current averaged edit-distance between lattice and R_.
    // \hat{L} in paper.

    std::vector<std::vector<std::pair<int32, float> > > gamma_;
    // The stats we accumulate; these are pairs of (posterior, word-id), and note
    // that word-id may be epsilon.

    // Appendix C of the paper.
    std::vector<std::vector<std::pair<float, float> > > times_;
    std::vector<std::pair<float, float> > sausage_times_;
    std::vector<std::pair<float, float> > one_best_times_;

    std::vector<float> one_best_confidences_;

};

#endif // KD_MBR_H
