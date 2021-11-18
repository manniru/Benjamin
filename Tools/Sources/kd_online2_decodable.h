#ifndef KD_ONLINE2_DECODABLE_H
#define KD_ONLINE2_DECODABLE_H

#include "bt_config.h"

#include "itf/online-feature-itf.h"
#include "matrix/matrix-lib.h"
#include "itf/decodable-itf.h"
#include "gmm/am-diag-gmm.h"
#include "hmm/transition-model.h"
#include "online2/online-gmm-decoding.h"
#include "kd_online2_model.h"
#include "online/online-feat-input.h"

class KdOnline2Decodable : public kaldi::DecodableInterface
{
public:
    KdOnline2Decodable(KdOnline2Model *mdl,
                       float scale, kaldi::Matrix<kaldi::BaseFloat> transform);


    /// Returns the scaled log likelihood
    virtual kaldi::BaseFloat LogLikelihood(int32 frame, int32 index);

    virtual bool IsLastFrame(int32 frame) const;

    virtual int32 NumFramesReady() const;

    /// Indices are one-based!  This is for compatibility with OpenFst.
    virtual int32 NumIndices() const;

private:
    void CacheFrame(int32 frame);

    kaldi::OnlineFeaturePipeline *features;
    kaldi::AmDiagGmm *ac_model;
    float ac_scale_;
    kaldi::TransitionModel *trans_model;
    int32 feat_dim;  // dimensionality of the input features
    kaldi::Vector<float> cur_feats_;
    int32 cur_frame_;
    std::vector<std::pair<int32, float> > cache_;
};


#endif // KD_ONLINE2_DECODABLE_H
