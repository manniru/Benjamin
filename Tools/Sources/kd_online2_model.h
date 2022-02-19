#ifndef KD_ONLINE2_MODEL_H
#define KD_ONLINE2_MODEL_H

#include "bt_config.h"

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <string>
#include <vector>

#include <gmm/am-diag-gmm.h>
#include <hmm/transition-model.h>
#include <util/kaldi-io.h>

#include "bt_config.h"
#include "backend.h"
class KdOnline2Model
{
public:
    KdOnline2Model(kaldi::TransitionModel *tran, kaldi::AmDiagGmm *acc_model,
                   std::string model_filename);

    kaldi::TransitionModel* GetTransitionModel();
    kaldi::AmDiagGmm* GetOnlineAlignmentModel();
    kaldi::AmDiagGmm* GetModel();
    kaldi::AmDiagGmm* GetFinalModel();

    kaldi::TransitionModel *t_model; // trained with online-CMVN features
    kaldi::AmDiagGmm *oa_model; //online alignment model;
private:
    // The ML-trained model used to get transforms (required)
    kaldi::AmDiagGmm *a_model;
};

#endif // KD_ONLINE2_MODEL_H
