#ifndef KD_DELTA_H
#define KD_DELTA_H

#include "kd_cmvn_state.h"
#include "matrix/matrix-lib.h"
#include "util/common-utils.h"
#include "bt_cfb.h"

#define KD_DELTA_WINDOW 2 // should be  0<window<1000

class KdDelta
{
public:
    explicit KdDelta(BtCFB *feat);

    void calcCoeffs();
    void Process(int frame, int max_frame);
    void applyCoeff(double *i_data, double coeff, double *o_data);
    void resetDelta(BtFrameBuf *buf);

private:
    int sumof2N2(int n);
    BtCFB *feature;
    std::vector<kaldi::Vector<float> > coeffs; // coeff to compute smooth derivative
};

#endif // KD_DELTA_H
