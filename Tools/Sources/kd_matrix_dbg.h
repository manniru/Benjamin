#ifndef KD_MATRIX_DBG_H
#define KD_MATRIX_DBG_H

#include <QObject>
#include "matrix/matrix-lib.h"

void kd_printMat(kaldi::Matrix<double> stats);
void kd_printMat2(kaldi::MatrixBase<float> *stats);

#endif // KD_MATRIX_DBG_H
