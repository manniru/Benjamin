#ifndef KD_ONLINE2_DECODABLE_H
#define KD_ONLINE2_DECODABLE_H

#include "bt_config.h"

//#ifdef BT_ONLINE2
#include "itf/online-feature-itf.h"
#include "matrix/matrix-lib.h"
#include "itf/decodable-itf.h"
#include "gmm/am-diag-gmm.h"
#include "hmm/transition-model.h"
#include "online2/online-gmm-decoding.h"

class KdOnline2Decodable : public kaldi::DecodableInterface
{
public:
    KdOnline2Decodable(kaldi::OnlineGmmDecodingModels *model,
                       float scale, kaldi::OnlineFeatureInterface *input_feats);


    /// Returns the scaled log likelihood
    virtual kaldi::BaseFloat LogLikelihood(int32 frame, int32 index);

    virtual bool IsLastFrame(int32 frame) const;

    virtual int32 NumFramesReady() const;

    /// Indices are one-based!  This is for compatibility with OpenFst.
    virtual int32 NumIndices() const;

private:
    void CacheFrame(int32 frame);

    kaldi::OnlineFeatureInterface *features;
    kaldi::AmDiagGmm *ac_model;
    float ac_scale_;
    kaldi::TransitionModel *trans_model;
    const int32 feat_dim_;  // dimensionality of the input features
    kaldi::Vector<float> cur_feats_;
    int32 cur_frame_;
    std::vector<std::pair<int32, float> > cache_;
};

//#endif

#endif // KD_ONLINE2_DECODABLE_H
