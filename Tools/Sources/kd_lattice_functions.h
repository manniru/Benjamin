#ifndef KD_LATTICE_FUNCTIONS_H
#define KD_LATTICE_FUNCTIONS_H

#include <QDebug>
#include "bt_config.h".
#include "kd_token2.h"

#include <vector>
#include <map>

#include "base/kaldi-common.h"
#include "hmm/posterior.h"
#include "fstext/fstext-lib.h"
#include "hmm/transition-model.h"
#include "lat/kaldi-lattice.h"
#include "itf/decodable-itf.h"

void kd_latticeGetTimes(kaldi::Lattice *lat, std::vector<int> *times);

#endif // KD_LATTICE_FUNCTIONS_H
