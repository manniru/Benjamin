#ifndef KD_MATRIX_H
#define KD_MATRIX_H

class KdMatrix
{
public:
    KdMatrix();
    ~KdMatrix();

    void free();
    void resize(int row, int column);
    void setZero();

    int    cols;
    int    rows;
    float **d;
};

#endif // KD_MATRIX_H
