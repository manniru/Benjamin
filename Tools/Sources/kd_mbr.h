#ifndef KD_MBR_H
#define KD_MBR_H

#define KD_MBR_DELTA 1.0e-05 // Defined in paper

// described in "Minimum Bayes Risk decoding and system combination based on a recursion for
// edit distance", Haihua Xu, Daniel Povey, Lidia Mangu and Jie Zhu,

#include <QString>
#include "backend.h"
#include "kd_mbr_base.h"
#include "kd_lattice_compact.h"
#include "kd_fst_util.h"

class KdMBR
{
public:
    KdMBR();

    void compute(KdCompactLattice *clat_in);
    QVector<BtWord>    getResult();

private:
    void checkLattice(KdCompactLattice *clat);
    void createTimes (KdCompactLattice *clat);
    void getBestWords(KdCompactLattice *clat);
    void createMBRLat(KdCompactLattice *clat);
    void MbrDecode();

    double l_distance(int a, int b, bool penalize = false);

    double EditDistance(int N, int Q,
                        kaldi::Vector<double> &alpha,
                        kaldi::Matrix<double> &alpha_dash,
                        kaldi::Vector<double> &alpha_dash_arc);

    void computeGamma();

    void RemoveEps(std::vector<int> *vec);
    void AddEpsBest();

    void AddToMap(int i, double d, std::map<int, double> *gamma);

    int max_state = 0;
    QVector<QString> lexicon;

    // Arcs in the topologically sorted acceptor form of the word-level lattice,
    // with one final-state
    std::vector<KdMBRArc> mlat_arc;

    // For each node in the lattice, a list of arcs entering that node.
    std::vector<std::vector<int> > mlat;

    std::vector<int> one_best_id; // R in paper
    double L_; // current averaged edit-distance between lattice and one_best_id.

    std::vector<std::vector<std::pair<int, float> > > gamma_;
    // The stats we accumulate; pair of (1.word-id, 2.posterior)

    // Appendix C of the paper.
    // may overlap.
    std::vector<std::vector<std::pair<float, float> > > times_;
    // do not overlap
    std::vector<std::pair<float, float> > sausage_times_;
    std::vector<std::pair<float, float> > one_best_times;
    std::vector<int> s_times; // time of each state in lattice,
    QVector<int> b_times; // time with benjamin flavour

    std::vector<float> one_best_conf;
};

#endif // KD_MBR_H
