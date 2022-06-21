#ifndef KD_A_MODEL_H
#define KD_A_MODEL_H

#include "util/kaldi-io.h"
#include "base/kaldi-common.h"
#include "bt_cfb.h"
#include "kd_gmm.h"


class KdAModel
{
public:
    KdAModel();
    ~KdAModel();

    float LogLikelihood(int32 pdf_index, BtFrameBuf *buf);
    void Read(std::istream &in_stream);

private:
    std::vector<KdGmm*> densities;
};


#endif // KD_A_MODEL_H
