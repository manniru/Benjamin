#ifndef KD_ONLINE2_DECODABLE_H
#define KD_ONLINE2_DECODABLE_H

#include "bt_config.h"

#ifdef BT_ONLINE2
#include "itf/online-feature-itf.h"
#include "matrix/matrix-lib.h"
#include "itf/decodable-itf.h"
#include "gmm/am-diag-gmm.h"
#include "hmm/transition-model.h"

namespace kaldi {


class KdOnline2Decodable : public DecodableInterface
{
public:
    KdOnline2Decodable(const AmDiagGmm &am,
                                 const TransitionModel &trans_model,
                                 const BaseFloat scale,
                                 OnlineFeatureInterface *input_feats);


    /// Returns the scaled log likelihood
    virtual BaseFloat LogLikelihood(int32 frame, int32 index);

    virtual bool IsLastFrame(int32 frame) const;

    virtual int32 NumFramesReady() const;

    /// Indices are one-based!  This is for compatibility with OpenFst.
    virtual int32 NumIndices() const { return trans_model_.NumTransitionIds(); }

private:
    void CacheFrame(int32 frame);

    OnlineFeatureInterface *features_;
    const AmDiagGmm &ac_model_;
    BaseFloat ac_scale_;
    const TransitionModel &trans_model_;
    const int32 feat_dim_;  // dimensionality of the input features
    Vector<BaseFloat> cur_feats_;
    int32 cur_frame_;
    std::vector<std::pair<int32, BaseFloat> > cache_;
};

} // namespace kaldi
#endif

#endif // KD_ONLINE2_DECODABLE_H
