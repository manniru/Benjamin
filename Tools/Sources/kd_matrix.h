#ifndef KD_MATRIX_H
#define KD_MATRIX_H

#include "base/kaldi-common.h"

class KdMatrix
{
public:
    KdMatrix();
    ~KdMatrix();

    void free();
    void resize(int row, int column);

    int    cols;
    int    rows;
    float **d;
};

#endif // KD_MATRIX_H
