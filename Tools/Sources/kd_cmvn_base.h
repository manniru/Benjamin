#ifndef KD_CMVN_BASE_H
#define KD_CMVN_BASE_H

#include <QObject>
#include <QThread>

#include "kd_cmvn_state.h"
#include "matrix/matrix-lib.h"
#include "util/common-utils.h"
#include "base/kaldi-error.h"

void kd_applyCmvn(const kaldi::MatrixBase<double> &stats,
               bool var_norm,
               kaldi::MatrixBase<float> *feats);

#endif // KD_CMVN_BASE_H
