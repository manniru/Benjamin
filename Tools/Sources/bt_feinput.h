#ifndef BT_FEINPUT_H
#define BT_FEINPUT_H

#include "bt_config.h"
#include "bt_recorder.h"
#include "bt_cmvn.h"
#include "kd_a_model.h"
#include "bt_mfcc.h"
#include "kd_delta.h"
#include "bt_cfb.h"

// Feature Input
class BtFeInput: public QObject
{
    Q_OBJECT
public:
    explicit BtFeInput(BtCyclic *buf,
                       QObject *parent = nullptr);
    ~BtFeInput();

    uint  NumFramesReady();
    void computeFrame(uint frame);

    void ComputeFeatures();

    BtCFB  *o_features;
private:
    BtMFCC *mfcc;
    BtCMVN *cmvn;

    uint frame_num;
    int  remain_samp = 0;
    kaldi::Vector<float> wav_buf;
    float window_buf[BT_FFT_SIZE];

    KdDelta    *delta;
    BtCyclic   *rec_buf;
};

#endif // BT_FEINPUT_H
