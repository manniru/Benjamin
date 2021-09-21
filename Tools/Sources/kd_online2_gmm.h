#ifndef KD_ONLINE2_GMM_H
#define KD_ONLINE2_GMM_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <string>
#include <vector>

#include "matrix/matrix-lib.h"
#include "util/common-utils.h"
#include "base/kaldi-error.h"
#include "transform/basis-fmllr-diag-gmm.h"
#include "transform/fmllr-diag-gmm.h"
#include "online2/online-feature-pipeline.h"
#include "online2/online-gmm-decodable.h"
#include "online2/online-endpoint.h"
#include "decoder/lattice-faster-online-decoder.h"
#include "hmm/transition-model.h"
#include "gmm/am-diag-gmm.h"
#include "hmm/posterior.h"
#include "online2/online-gmm-decoding.h"
#include "bt_config.h"



class KdOnline2Gmm : public QObject
{
    Q_OBJECT
public:
    explicit KdOnline2Gmm(QObject *parent = NULL);
    ~KdOnline2Gmm();

    kaldi::OnlineFeaturePipeline &FeaturePipeline() { return *feature_pipeline_; }
    void AdvanceDecoding();
    void FinalizeDecoding();
    void EstimateFmllr(bool end_of_utterance);

    void GetAdaptationState(kaldi::OnlineGmmAdaptationState *adaptation_state);
    void GetLattice(bool rescore_if_needed,
                    bool end_of_utterance,
                    kaldi::CompactLattice *clat);

    void init();
    kaldi::LatticeFasterOnlineDecoder *decoder_;

private:
    bool GetGaussianPosteriors(bool end_of_utterance, kaldi::GaussPost *gpost);
    bool HaveTransform() const;
    bool RescoringIsNeeded();


    kaldi::OnlineGmmDecodingConfig d_config;
    kaldi::OnlineGmmDecodingModels *models_;
    kaldi::OnlineFeaturePipeline *feature_pipeline_;  // owned here.
    kaldi::OnlineGmmAdaptationState orig_adaptation_state_;
    kaldi::OnlineGmmAdaptationState adaptation_state_;

    std::vector<int> silence_phones_; // sorted, unique list of silence phones,
                                        // derived from d_config
};

#endif // KD_ONLINE2_GMM_H
