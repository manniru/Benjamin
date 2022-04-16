#ifndef KD_MODEL_H
#define KD_MODEL_H

#include "bt_config.h"

#include <QObject>
#include <QDebug>

#include <gmm/am-diag-gmm.h>
#include <hmm/transition-model.h>
#include <util/kaldi-io.h>

#include "bt_config.h"
#include "backend.h"
class KdModel
{
public:
    KdModel(std::string model_filename);
    ~KdModel();
};

#endif // KD_MODEL_H
