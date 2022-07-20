#ifndef KD_MBR_H
#define KD_MBR_H

// paper "MBR decoding and system based on a recursion for edit distance"

#include <QString>
#include "kd_levenshtein.h"
#include "backend.h"
#include "kd_matrix.h"
#include "bt_state.h"

class KdMBR
{
public:
    KdMBR(BtState *state);

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
    QStringList lexicon;

    std::vector<KdMBRArc> mlat_arc;
    std::vector<std::vector<int> > mlat;

    std::vector<int> best_wid; // R in paper
    double L_; // current averaged edit-distance between lattice and one_best_id.

    KdGammaVec lat_gamma;

    QVector<int> b_times; // time with benjamin flavour
    std::vector<float> best_conf;

    QVector<double> alpha;
    QVector<double> alpha_dash_arc;
    KdMatrix alpha_dash;
};
#define KD_DBL_EPSILON 2.2204460492503131e-16
#define KD_DIFF_DOUBLE log(KD_DBL_EPSILON)
double kd_LogAdd(double x, double y);

#endif // KD_MBR_H
