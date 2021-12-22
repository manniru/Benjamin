#ifndef KD_ONLINE_DECODABLE_H
#define KD_ONLINE_DECODABLE_H

#include "online/online-decodable.h"

#include "bt_config.h"
#include "bt_recorder.h"
#include "kd_online_feinput.h"
#include "kd_online2_model.h"

#define KD_FRAME_LENGTH 20 //ms
#define KD_FRAME_SHIFT  5  //ms
#define KD_WINDOW_SIZE BT_REC_RATE/1000*KD_FRAME_LENGTH
#define KD_FEAT_SIZE   BT_REC_RATE/1000*KD_FRAME_SHIFT

// A decodable, taking input from an OnlineFeatureInput object on-demand
class KdOnlineDecodable: public kaldi::DecodableInterface
{
public:
    KdOnlineDecodable(BtRecorder *au_src, KdOnline2Model *mdl,
                      float scale);
    ~KdOnlineDecodable();


    /// Returns the log likelihood, which will be negated in the decoder.
    virtual kaldi::BaseFloat LogLikelihood(int32 frame, int32 index);

    virtual bool IsLastFrame(int32 frame) const;

    /// Indices are one-based!  This is for compatibility with OpenFst.
    virtual int32 NumIndices() const { return trans_model->NumTransitionIds(); }

    kaldi::OnlineFeatureMatrix *features;
    kaldi::OnlineCmnInput *cmn_input;
    KdOnlineFeInput *fe_input;
    kaldi::Mfcc *mfcc;
private:
    void CacheFrame(int32 frame);

    kaldi::AmDiagGmm *ac_model;
    float ac_scale_;
    kaldi::TransitionModel *trans_model;
    int32 feat_dim;  // dimensionality of the input features
    kaldi::Vector<float> cur_feats_;
    int32 cur_frame_;
    std::vector<std::pair<int32, float> > cache_;

    int kDeltaOrder = 2; // delta-delta derivative order
    int cmn_window = 600, min_cmn_window = 100;
    kaldi::OnlineFeatInputItf    *feat_transform;

};

#endif // KD_ONLINE_DECODABLE_H
