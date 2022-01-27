#ifndef KD_DELTA_H
#define KD_DELTA_H

#include "kd_cmvn_state.h"
#include "matrix/matrix-lib.h"
#include "util/common-utils.h"

class KdDelta
{
public:
    // The function takes as input a matrix of features and a frame index
    // that it should compute the deltas on.  It puts its output in an object
    // of type VectorBase, of size (original-feature-dimension) * (opts.order+1).

    explicit KdDelta();

    void Process(const kaldi::MatrixBase<float> &input_feats,
                 int32 frame,
                 kaldi::VectorBase<float> *output_frame) const;

    int32 order = 2;
    int32 window = 2;  // e.g. 2; controls window size (window size is 2*window + 1)
    // the behavior at the edges is to replicate the first or last frame.
private:
    std::vector<kaldi::Vector<float> > scales_;  // a scaling window for each
    // of the orders, including zero: multiply the features for each
    // dimension by this window.
};

#endif // KD_DELTA_H
