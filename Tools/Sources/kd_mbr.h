#ifndef KD_MBR_H
#define KD_MBR_H

#include "lat/sausages.h"
#include "lat/lattice-functions.h"

class KdMBR
{
public:
    KdMBR(kaldi::CompactLattice *clat,
          kaldi::MinimumBayesRiskOptions opts);

    const std::vector<int32> &GetOneBest() const { // gets one-best (with no epsilons)
        return R_;
    }

    const std::vector<std::vector<std::pair<float, float> > > GetTimes() const {
        return times_; // returns average (start,end) times for each word in each
        // bin. These are raw averages without any processing, i.e. time intervals
        // from different bins can overlap.
    }

    const std::vector<std::pair<float, float> > GetSausageTimes() const {
        return sausage_times_; // returns average (start,end) times for each bin.
        // This is typically the weighted average of the times in GetTimes() but can
        // be slightly different if the times for the bins overlap, in which case
        // the times returned by this method do not overlap unlike the times
        // returned by GetTimes().
    }

    const std::vector<std::pair<float, float> > &GetOneBestTimes() const {
        return one_best_times_; // returns average (start,end) times for each word
        // corresponding to an entry in the one-best output.  This is typically the
        // appropriate subset of the times in GetTimes() but can be slightly
        // different if the times for the one-best words overlap, in which case
        // the times returned by this method do not overlap unlike the times
        // returned by GetTimes().
    }

    /// Outputs the confidences for the one-best transcript.
    const std::vector<float> &GetOneBestConfidences() const {
        return one_best_confidences_;
    }

    /// Returns the expected WER over this sentence (assuming model correctness).
    float GetBayesRisk() const { return L_; }

    const std::vector<std::vector<std::pair<int32, float> > > &GetSausageStats() const {
        return gamma_;
    }

private:
    void PrepareLatticeAndInitStats(kaldi::CompactLattice *clat);

    /// Minimum-Bayes-Risk Decode. Top-level algorithm.  Figure 6 of the paper.
    void MbrDecode();

    /// Without the 'penalize' argument this gives us the basic edit-distance
    /// function l(a,b), as in the paper.
    /// With the 'penalize' argument it can be interpreted as the edit distance
    /// plus the 'delta' from the paper, except that we make a kind of conceptual
    /// bug-fix and only apply the delta if the edit-distance was not already
    /// zero.  This bug-fix was necessary in order to force all the stats to show
    /// up, that should show up, and applying the bug-fix makes the sausage stats
    /// significantly less sparse.
    inline double l(int32 a, int32 b, bool penalize = false) {
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
    static inline void AddToMap(int32 i, double d, std::map<int32, double> *gamma) {
        if (d == 0) return;
        std::pair<const int32, double> pr(i, d);
        std::pair<std::map<int32, double>::iterator, bool> ret = gamma->insert(pr);
        if (!ret.second) // not inserted, so add to contents.
            ret.first->second += d;
    }

    struct Arc {
        int32 word;
        int32 start_node;
        int32 end_node;
        float loglike;
    };

    kaldi::MinimumBayesRiskOptions opts_;


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
    // that word-id may be epsilon.  Caution: indexed from zero, not from 1 as in
    // paper.  We sort in reverse order on the second member (posterior), so more
    // likely word is first.

    std::vector<std::vector<std::pair<float, float> > > times_;
    // The average start and end times for words in each confusion-network bin.
    // This is like an average over arcs, of the tau_b and tau_e quantities in
    // Appendix C of the paper.  Indexed from zero, like gamma_ and R_.

    std::vector<std::pair<float, float> > sausage_times_;
    // The average start and end times for each confusion-network bin.  This
    // is like an average over words, of the tau_b and tau_e quantities in
    // Appendix C of the paper.  Indexed from zero, like gamma_ and R_.

    std::vector<std::pair<float, float> > one_best_times_;
    // The average start and end times for words in the one best output.  This
    // is like an average over the arcs, of the tau_b and tau_e quantities in
    // Appendix C of the paper. Indexed from zero, like gamma_ and R_.

    std::vector<float> one_best_confidences_;
    // vector of confidences for the 1-best output (which could be
    // the MAP output if opts_.decode_mbr == false, or the MBR output otherwise).
    // Indexed by the same index as one_best_times_.

    struct GammaCompare{
        // should be like operator <.  But we want reverse order
        // on the 2nd element (posterior), so it'll be like operator
        // > that looks first at the posterior.
        bool operator () (const std::pair<int32, float> &a,
                          const std::pair<int32, float> &b) const {
            if (a.second > b.second) return true;
            else if (a.second < b.second) return false;
            else return a.first > b.first;
        }
    };
};

#endif // KD_MBR_H
