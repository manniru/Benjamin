#ifndef KD_ONLINE2_DECODABLE_H
#define KD_ONLINE2_DECODABLE_H

#include "bt_config.h"

#include "matrix/matrix-lib.h"
#include "gmm/am-diag-gmm.h"
#include "hmm/transition-model.h"
#include "kd_online2_model.h"
#include "kd_online2_feinput.h"

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

class KdOnline2Decodable
{
public:
    KdOnline2Decodable(BtRecorder *au_src, KdOnline2Model *mdl, float scale);
    ~KdOnline2Decodable();

    /// Returns the scaled log likelihood
    float LogLikelihood(int frame, int index);
    int NumFramesReady();
    void addPDF(int frame, int id, int phone_id, float val);

    int NumIndices();
    KdOnline2FeInput *features;
    int max_pdf;

    QVector<QVector<KdPDF *>> p_vec;

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


#endif // KD_ONLINE2_DECODABLE_H
