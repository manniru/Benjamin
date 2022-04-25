#ifndef KD_MBR_H
#define KD_MBR_H

// paper "MBR decoding and system based on a recursion for edit distance"

#include <QString>
#include "kd_levenshtein.h"
#include "backend.h"
#include "matrix/kaldi-matrix.h"

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

    double editDistance();

    int max_state = 0;
    QVector<QString> lexicon;

    std::vector<KdMBRArc> mlat_arc;
    std::vector<std::vector<int> > mlat;

    std::vector<int> best_wid; // R in paper
    double L_; // current averaged edit-distance between lattice and one_best_id.

    KdGammaVec lat_gamma;

    QVector<int> b_times; // time with benjamin flavour
    std::vector<float> best_conf;

    QVector<double> alpha;
    QVector<double> alpha_dash_arc;
    kaldi::Matrix<double> alpha_dash;
};

#endif // KD_MBR_H
