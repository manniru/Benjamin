#ifndef KD_MBR_H
#define KD_MBR_H

#define KD_MBR_DELTA 1.0e-05 // Defined in paper

// described in "Minimum Bayes Risk decoding and system combination based on a recursion for
// edit distance", Haihua Xu, Daniel Povey, Lidia Mangu and Jie Zhu,

#include "lat/sausages.h"
#include "lat/lattice-functions.h"
#include <QString>


struct KdMBRArc
{
    int32 word;
    int32 start_node;
    int32 end_node;
    float loglike;
};

typedef struct BtWord
{
    QString word;
    double  time;
    double  start;
    double  end;
    double  conf;
}BtWord;

class KdMBR
{
public:
    KdMBR(kaldi::CompactLattice *clat);

    std::vector<int32> GetOneBest();
    std::vector<std::vector<std::pair<float, float> > > GetTimes();
    std::vector<std::pair<float, float> > GetSausageTimes();
    std::vector<std::pair<float, float> > GetOneBestTimes();
    std::vector<float> GetOneBestConfidences();
    QVector<BtWord>    getResult(QVector<QString> lexicon);

private:
    void PrepareLatticeAndInitStats(kaldi::CompactLattice *clat);

    void MbrDecode(); // Figure 6 of the paper.

    // gives edit-distance function l(a,b)
    double l_distance(int32 a, int32 b, bool penalize = false);

    /// Figure 4 of the paper; called from AccStats (Fig. 5)
    double EditDistance(int32 N, int32 Q,
                        kaldi::Vector<double> &alpha,
                        kaldi::Matrix<double> &alpha_dash,
                        kaldi::Vector<double> &alpha_dash_arc);

    /// Figure 5 of the paper.  Outputs to gamma_ and L_.
    void AccStats();

    /// Removes epsilons (symbol 0) from a vector
    void RemoveEps(std::vector<int32> *vec);

    // Ensures that between each word in "vec" and at the beginning and end, is
    // epsilon (0).  (But if no words in vec, just one epsilon)
    void NormalizeEps(std::vector<int32> *vec);


    void AddToMap(int32 i, double d, std::map<int32, double> *gamma);

    kaldi::MinimumBayesRiskOptions opts;

    /// Arcs in the topologically sorted acceptor form of the word-level lattice,
    /// with one final-state.  Contains (word-symbol, log-likelihood on arc ==
    /// negated cost).  Indexed from zero.
    std::vector<KdMBRArc> arcs_;

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
