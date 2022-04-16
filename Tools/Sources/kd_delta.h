#ifndef KD_DELTA_H
#define KD_DELTA_H

#include "bt_cfb.h"

#define KD_DELTA_WINDOW 2 // should be  0<window<1000

class KdDelta
{
public:
    explicit KdDelta(BtCFB *feat);

    void calcCoeffs();
    void Process(uint frame, int max_frame);
    void applyCoeff(double *i_data, double coeff, double *o_data);
    void resetDelta(BtFrameBuf *buf);

private:
    int sumof2N2(int n);
    BtCFB *feature;
    QVector<float> coeffs[BT_DELTA_ORDER+1]; // coeff to compute smooth derivative
};

#endif // KD_DELTA_H
