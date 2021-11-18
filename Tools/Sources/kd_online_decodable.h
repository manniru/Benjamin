#ifndef KD_ONLINE_DECODABLE_H
#define KD_ONLINE_DECODABLE_H

#include "online/online-decodable.h"

#include "bt_config.h"
#include "kd_online2_model.h"

// A decodable, taking input from an OnlineFeatureInput object on-demand
class KdOnlineDecodable: public kaldi::DecodableInterface
{
public:
    KdOnlineDecodable(KdOnline2Model *mdl,
                      float scale, kaldi::OnlineFeatureMatrix *input_feats);


    /// Returns the log likelihood, which will be negated in the decoder.
    virtual kaldi::BaseFloat LogLikelihood(int32 frame, int32 index);

    virtual bool IsLastFrame(int32 frame) const;

    /// Indices are one-based!  This is for compatibility with OpenFst.
    virtual int32 NumIndices() const { return trans_model->NumTransitionIds(); }

private:
    void CacheFrame(int32 frame);

    kaldi::OnlineFeatureMatrix *features;
    kaldi::AmDiagGmm *ac_model;
    float ac_scale_;
    kaldi::TransitionModel *trans_model;
    int32 feat_dim;  // dimensionality of the input features
    kaldi::Vector<float> cur_feats_;
    int32 cur_frame_;
    std::vector<std::pair<int32, float> > cache_;

};

#endif // KD_ONLINE_DECODABLE_H
