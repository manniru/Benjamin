#ifndef KD_LATTICE_PRUNE_H
#define KD_LATTICE_PRUNE_H

#include "kd_lattice.h"
#include "kd_token.h"

#define KD_LAT_BEAM 6.0

class KdPrune
{
public:
    KdPrune(float beam = KD_LAT_BEAM);
    QString prune(KdLattice *lat);

    float lattice_beam;
private:
    double getCutOff(KdLattice *lat);

    int num_states;
    QVector<double> forward_cost;  // viterbi forward.
};

#endif // KD_LATTICE_PRUNE_H
