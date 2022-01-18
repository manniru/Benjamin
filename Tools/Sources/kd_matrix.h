#ifndef KD_MATRIX_H
#define KD_MATRIX_H

#include "util/kaldi-io.h"

void kd_readMatrix(std::string &filename, kaldi::Matrix<float> *c);

#endif // KD_MATRIX_H
