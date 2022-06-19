#ifndef BT_FEINPUT_H
#define BT_FEINPUT_H

#include "config.h"
#include "bt_recorder.h"
#include "bt_cmvn.h"
#include "kd_a_model.h"
#include "bt_mfcc.h"
#include "bt_delta.h"
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
    void enableENN();

    BtCFB   *o_features;
    BtCMVN  *cmvn;
    KdDelta *delta;
private:
    BtMFCC *mfcc;
    BtMFCC *mfcc_enn = NULL;

    uint frame_num;
    int  remain_samp = 0;
    float window_buf[BT_FFT_SIZE];

    BtCyclic   *rec_buf;
};

#endif // BT_FEINPUT_H
