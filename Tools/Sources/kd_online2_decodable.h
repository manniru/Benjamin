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
#include "kd_online2_feinput.h"


class KdOnline2Decodable
{
public:
    KdOnline2Decodable(BtRecorder *au_src, KdOnline2Model *mdl, float scale);
    ~KdOnline2Decodable();


    /// Returns the scaled log likelihood
    float LogLikelihood(int32 frame, int index);

    int NumFramesReady();

    /// Indices are one-based!  This is for compatibility with OpenFst.
    int NumIndices();
    KdOnline2FeInput *features;

private:
    void CacheFrame(int frame);

    kaldi::AmDiagGmm *ac_model;
    float ac_scale_;
    kaldi::TransitionModel *trans_model;
    int32 feat_dim;  // dimensionality of the input features
    kaldi::Vector<float> cur_feats_;
    int32 cur_frame_;
    std::vector<std::pair<int32, float> > cache_;
};


#endif // KD_ONLINE2_DECODABLE_H
