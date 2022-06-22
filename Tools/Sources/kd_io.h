#ifndef KD_IO_H
#define KD_IO_H

#include "base/kaldi-common.h"
#include "matrix/matrix-lib.h"

typedef struct KdMat
{
    int    cols;
    int    rows;
    float **d;
} KdMat;

std::vector<float> kd_VectorRead(std::istream &is);
KdMat kd_MatrixRead(std::istream &is);

#endif // KD_IO_H
