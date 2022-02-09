#ifndef KD_LATTICE_FUNCTIONS_H
#define KD_LATTICE_FUNCTIONS_H

#include <QDebug>
#include "bt_config.h".
#include "kd_token2.h"

#include <vector>
#include <map>

#include "base/kaldi-common.h"
#include "hmm/posterior.h"
#include "hmm/transition-model.h"
#include "lat/kaldi-lattice.h"
#include "kd_lattice.h"

void kd_latticeGetTimes(KdLattice *lat, std::vector<int> *times);
bool kd_PruneLattice(float beam, KdLattice *lat);

#endif // KD_LATTICE_FUNCTIONS_H
