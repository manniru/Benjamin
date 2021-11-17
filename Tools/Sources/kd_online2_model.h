#ifndef KD_ONLINE2_MODEL_H
#define KD_ONLINE2_MODEL_H

#include "bt_config.h"

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <string>
#include <vector>

#include "online2/online-gmm-decoding.h"

#include "bt_config.h"
#include "backend.h"
class KdOnline2Model
{
public:
    KdOnline2Model(kaldi::TransitionModel *tran, kaldi::AmDiagGmm *acc_model,
                   kaldi::OnlineGmmDecodingConfig *config);

    kaldi::TransitionModel* GetTransitionModel();

    kaldi::AmDiagGmm* GetOnlineAlignmentModel();

    kaldi::AmDiagGmm* GetModel();

    kaldi::AmDiagGmm* GetFinalModel();

    kaldi::BasisFmllrEstimate* GetFmllrBasis();

private:
    kaldi::TransitionModel *t_model;
    // The model trained with online-CMVN features
    kaldi::AmDiagGmm *oa_model; //online alignment model;
    // The ML-trained model used to get transforms (required)
    kaldi::AmDiagGmm *a_model;
    // The following object contains the basis elements for
    // "Basis fMLLR".
    kaldi::BasisFmllrEstimate *fmllr_basis;
};

#endif // KD_ONLINE2_MODEL_H
