#ifndef KD_IO_H
#define KD_IO_H

#include "base/kaldi-common.h"
#include "matrix/matrix-lib.h"
#include "kd_matrix.h"

std::vector<float> kd_VectorRead(std::istream &is);
KdMatrix kd_MatrixRead(std::istream &is);

#endif // KD_IO_H
