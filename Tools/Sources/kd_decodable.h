#ifndef KD_DECODABLE_H
#define KD_DECODABLE_H

#include "bt_config.h"

#include "matrix/matrix-lib.h"
#include "gmm/am-diag-gmm.h"
#include "hmm/transition-model.h"
#include "kd_online2_model.h"
#include "kd_online2_feinput.h"

#define MAX_FRAME_CNT 400
typedef struct KdPDF
{
    int pdf_id;
    int phone_id;
    float val;

    bool operator<(const KdPDF &b)
    {
        return val<b.val;
    }
} KdPDF;

// KdOnline2Decodable
class KdDecodable
{
public:
    KdDecodable(BtRecorder *au_src, KdOnline2Model *mdl, float scale);
    ~KdDecodable();

    /// Returns the scaled log likelihood
    float LogLikelihood(int frame, int index);
    int NumFramesReady();

    int NumIndices();
    KdOnline2FeInput *features;


private:
    void CacheFeature(int frame);

    kaldi::AmDiagGmm *ac_model;
    float ac_scale_;
    kaldi::TransitionModel *trans_model;
    int feat_dim;  // dimensionality of the input features
    kaldi::Vector<float> cur_feats_;
    int cur_frame_;
    std::vector<std::pair<int, float> > cache_;
};


#endif // KD_DECODABLE_H