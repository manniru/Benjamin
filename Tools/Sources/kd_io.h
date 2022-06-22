#ifndef KD_IO_H
#define KD_IO_H

#include "base/kaldi-common.h"
#include "matrix/matrix-lib.h"

std::vector<float> kd_VectorRead(std::istream &is);
kaldi::Matrix<float> kd_MatrixRead(std::istream &is);

#endif // KD_IO_H
