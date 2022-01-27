#include "kd_delta.h"

using namespace kaldi;

KdDelta::KdDelta()
{
    KALDI_ASSERT(order >= 0 && order < 1000);  // just make sure we don't get binary junk.
    // opts will normally be 2 or 3.
    KALDI_ASSERT(window > 0 && window < 1000);  // again, basic sanity check.
    // normally the window size will be two.

    scales_.resize(order+1);
    scales_[0].Resize(1);
    scales_[0](0) = 1.0;  // trivial window for 0th order delta [i.e. baseline feats]

    for (int32 i = 1; i <= order; i++)
    {
        Vector<float> &prev_scales = scales_[i-1],
                &cur_scales = scales_[i];
        // work if instead we later make it an array and do window[i-1],
        // or something like that. "window" is a parameter specifying delta-window
        // width which is actually 2*window + 1.
        KALDI_ASSERT(window != 0);
        int32 prev_offset = (static_cast<int32>(prev_scales.Dim()-1))/2,
                cur_offset = prev_offset + window;
        cur_scales.Resize(prev_scales.Dim() + 2*window);  // also zeros it.

        float normalizer = 0.0;
        for (int32 j = -window; j <= window; j++) {
            normalizer += j*j;
            for (int32 k = -prev_offset; k <= prev_offset; k++) {
                cur_scales(j+k+cur_offset) +=
                        static_cast<float>(j) * prev_scales(k+prev_offset);
            }
        }
        cur_scales.Scale(1.0 / normalizer);
    }
}

void KdDelta::Process(const MatrixBase<float> &input_feats,
                      int32 frame,
                      VectorBase<float> *output_frame) const {
    KALDI_ASSERT(frame < input_feats.NumRows());
    int32 num_frames = input_feats.NumRows(),
            feat_dim = input_feats.NumCols();
    KALDI_ASSERT(static_cast<int32>(output_frame->Dim()) == feat_dim * (order+1));
    output_frame->SetZero();
    for (int32 i = 0; i <= order; i++) {
        const Vector<float> &scales = scales_[i];
        int32 max_offset = (scales.Dim() - 1) / 2;
        SubVector<float> output(*output_frame, i*feat_dim, feat_dim);
        for (int32 j = -max_offset; j <= max_offset; j++) {
            // if asked to read
            int32 offset_frame = frame + j;
            if (offset_frame < 0) offset_frame = 0;
            else if (offset_frame >= num_frames)
                offset_frame = num_frames - 1;
            float scale = scales(j + max_offset);
            if (scale != 0.0)
                output.AddVec(scale, input_feats.Row(offset_frame));
        }
    }
}
