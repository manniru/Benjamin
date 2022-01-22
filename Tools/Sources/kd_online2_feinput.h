#ifndef KD_ONLINE2_FEINPUT_H
#define KD_ONLINE2_FEINPUT_H

#include "bt_config.h"
#include "bt_recorder.h"
#include "kd_cmvn.h"
#include "kd_matrix.h"
#include "kd_mfcc.h"

class KdOnline2FeInput: public QObject
{
    Q_OBJECT
public:
    explicit KdOnline2FeInput(BtRecorder *au_src,
                              QObject *parent = nullptr);
    ~KdOnline2FeInput();

    int  Dim();
    int  NumFramesReady();
    void GetFrame(int frame, kaldi::VectorBase<float> *feat);
    void FreezeCmvn();

    void AcceptWaveform(BtCyclic *buf, int len=0);
    void ComputeFeatures();

private:
    void Init();

    kaldi::Matrix<float> global_cmvn_stats_;

    KdCMVN *cmvn;
    kaldi::DeltaFeaturesOptions delta_opts;

    kaldi::RecyclingVector *o_features;
    KdMFCC *mfcc;

    kaldi::Vector<float> waveform_remainder_;

    // number of samples that discarded and were prior to 'waveform_remainder_'
    int64 waveform_offset;

    kaldi::DeltaFeatures *delta_features;
    BtRecorder *rec_src;
    QThread    *rec_thread;
};

#endif // KD_ONLINE2_FEINPUT_H
