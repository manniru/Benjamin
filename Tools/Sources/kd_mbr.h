#ifndef KD_MBR_H
#define KD_MBR_H

// described in "Minimum Bayes Risk decoding and system combination based on a recursion for
// edit distance", Haihua Xu, Daniel Povey, Lidia Mangu and Jie Zhu,

#include <QString>
#include "kd_levenshtein.h"
#include "backend.h"

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
    void calculateConf();

    void computeGamma();
    void RemoveEps();
    void AddEpsBest();

    double editDistance(int N, int Q,
                        kaldi::Vector<double> &alpha,
                        kaldi::Matrix<double> &alpha_dash,
                        kaldi::Vector<double> &alpha_dash_arc);

    int max_state = 0;
    QVector<QString> lexicon;

    // Arcs in the topologically sorted acceptor form of the word-level lattice,
    // with one final-state
    std::vector<KdMBRArc> mlat_arc;

    // For each node in the lattice, a list of arcs entering that node.
    std::vector<std::vector<int> > mlat;

    std::vector<int> best_wid; // R in paper
    double L_; // current averaged edit-distance between lattice and one_best_id.

    KdGammaVec lat_gamma;

    QVector<int> b_times; // time with benjamin flavour
    std::vector<float> best_conf;
};

#endif // KD_MBR_H
