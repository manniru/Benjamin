#ifndef KD_ONLINE2_GMM_H
#define KD_ONLINE2_GMM_H

#include "bt_config.h"

#ifdef BT_ONLINE2
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
#include "kd_lattice_decoder.h"
#include "kd_online2_decodable.h"
#include "kd_online_decodable.h"
#include "kd_online_feinput.h"
#include <fst/fst.h>
#include "bt_config.h"
#include "backend.h"

#define  BT_ONLINE_DECODABLE KdOnline2Decodable

class KdOnline2Gmm : public QObject
{
    Q_OBJECT
public:
    explicit KdOnline2Gmm(QObject *parent = NULL);
    ~KdOnline2Gmm();

    void init(BtRecorder *au_src);
    void AdvanceDecoding();
    void FinalizeDecoding();

    bool GetLattice(bool end_of_utterance,
                    kaldi::CompactLattice *clat);

    KdLatticeDecoder   *o_decoder;
    BT_ONLINE_DECODABLE *decodable;

private:
    kaldi::OnlineGmmDecodingConfig d_config;
    KdOnline2Model *models;
    kaldi::OnlineGmmAdaptationState adaptation_state_;

    std::vector<int> silence_phones_; // sorted, unique list of silence phones,
                                      // derived from d_config
    fst::Fst<fst::StdArc> *decode_fst;
    kaldi::OnlineFeaturePipelineConfig *feature_config;
};
#endif

#endif // KD_ONLINE2_GMM_H
