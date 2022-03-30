#ifndef KD_FEINPUT_H
#define KD_FEINPUT_H

#include "bt_config.h"
#include "bt_recorder.h"
#include "kd_cmvn.h"
#include "kd_matrix.h"
#include "kd_mfcc.h"
#include "kd_delta.h"

// Feature Input
class KdFeInput: public QObject
{
    Q_OBJECT
public:
    explicit KdFeInput(BtRecorder *au_src,
                              QObject *parent = nullptr);
    ~KdFeInput();

    int  Dim();
    int  NumFramesReady();
    void GetFrame(int frame, kaldi::VectorBase<float> *feat);
    void resetCmvn();

    void AcceptWaveform(BtCyclic *buf, int len=0);
    void ComputeFeatures();

private:
    void Init();

    KdMFCC *mfcc;
    KdCMVN *cmvn;
    KdRecyclingVector *o_features;

    // number of samples that discarded and were prior to 'waveform_remainder_'
    int waveform_offset;
    kaldi::Vector<float> waveform_remainder_;

    KdDelta    *delta;
    BtRecorder *rec_src;
};

#endif // KD_FEINPUT_H
