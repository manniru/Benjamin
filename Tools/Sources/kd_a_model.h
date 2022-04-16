#ifndef KD_A_MODEL_H
#define KD_A_MODEL_H

#include "util/kaldi-io.h"
#include "base/kaldi-common.h"
#include "gmm/diag-gmm.h"
#include "bt_cfb.h"


class KdAModel
{
public:
    KdAModel();
    ~KdAModel();

    float LogLikelihood(int32 pdf_index, BtFrameBuf *buf);
    void Read(std::istream &in_stream, bool binary);

private:
    std::vector<kaldi::DiagGmm*> densities_;
};


#endif // KD_A_MODEL_H
