#ifndef KD_DELTA_H
#define KD_DELTA_H

#include "kd_cmvn_state.h"
#include "matrix/matrix-lib.h"
#include "util/common-utils.h"

class KdDelta
{
public:
    explicit KdDelta();

    // compute the deltas and output a VectorBase
    // of size (original-feature-dimension) * (opts.order+1).
    void Process(kaldi::MatrixBase<float> &input_feats,
                 int32 frame,
                 kaldi::VectorBase<float> *output_frame);

    int32 order = 2;
    int32 window = 2;  // e.g. 2; controls window size (window size is 2*window + 1)
private:
    std::vector<kaldi::Vector<float> > scales_;  // a scaling window for each
    // of the orders, including zero: multiply the features for each
    // dimension by this window.
};

#endif // KD_DELTA_H
