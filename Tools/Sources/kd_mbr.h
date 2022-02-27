#ifndef KD_MBR_H
#define KD_MBR_H

#define KD_MBR_DELTA 1.0e-05 // Defined in paper

// described in "Minimum Bayes Risk decoding and system combination based on a recursion for
// edit distance", Haihua Xu, Daniel Povey, Lidia Mangu and Jie Zhu,

#include <QString>
#include "backend.h"
#include "kd_mbr_base.h"
#include "kd_lattice_compact.h"

class KdMBR
{
public:
    KdMBR(KdCompactLattice *clat);

    QVector<BtWord>    getResult();

private:
    void PrepareLatticeAndInitStats(KdCompactLattice *clat);
    void MbrDecode();

    double l_distance(int a, int b, bool penalize = false);

    double EditDistance(int N, int Q,
                        kaldi::Vector<double> &alpha,
                        kaldi::Matrix<double> &alpha_dash,
                        kaldi::Vector<double> &alpha_dash_arc);

    void AccStats();

    void RemoveEps(std::vector<int> *vec);
    void NormalizeBest();

    void AddToMap(int i, double d, std::map<int, double> *gamma);

    KdMBROpt opts;
    QVector<QString> lexicon;

    // Arcs in the topologically sorted acceptor form of the word-level lattice,
    // with one final-state
    std::vector<KdMBRArc> arcs_;

    // For each node in the lattice, a list of arcs entering that node.
    // (first node == 1).
    std::vector<std::vector<int> > pre_;

    std::vector<int> one_best_id; // R in paper
    double L_; // current averaged edit-distance between lattice and one_best_id.

    std::vector<std::vector<std::pair<int, float> > > gamma_;
    // The stats we accumulate; these are pairs of (posterior, word-id), and note
    // that word-id may be epsilon.

    // Appendix C of the paper.
    // may overlap.
    std::vector<std::vector<std::pair<float, float> > > times_;
    // do not overlap
    std::vector<std::pair<float, float> > sausage_times_;
    std::vector<std::pair<float, float> > one_best_times;
    std::vector<int> state_times_; // time of each state in lattice,

    std::vector<float> one_best_conf;
};

#endif // KD_MBR_H
