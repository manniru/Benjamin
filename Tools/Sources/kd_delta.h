#ifndef KD_DELTA_H
#define KD_DELTA_H

#include "kd_cmvn_state.h"
#include "matrix/matrix-lib.h"
#include "util/common-utils.h"

#define KD_DELTA_WINDOW 2 // should be  0<window<1000
#define KD_DELTA_ORDER  2 // should be  0<=order<1000

class KdDelta
{
public:
    explicit KdDelta();

    // compute the deltas and output a VectorBase
    // of size (original-feature-dimension) * (opts.order+1).
    void Process(kaldi::MatrixBase<float> &input_feats,
                 int32 frame,
                 kaldi::VectorBase<float> *output_frame);

private:
    int sumof2N2(int n);
    std::vector<kaldi::Vector<float> > scales;  // a scaling window for each
    // of the orders
};

#endif // KD_DELTA_H
