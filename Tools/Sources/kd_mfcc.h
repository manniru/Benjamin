#ifndef KD_MFCC_H
#define KD_MFCC_H

#include <QString>
#include "kd_window.h"
#include "kd_melbank.h"
#include "kd_fft.h"
#include "bt_cfb.h"

// must be >= BT_FEAT_SIZE
#define BT_MFCC_BIN 23 // 23 for 16khz, 15 for 8khz

class KdMFCC
{
public:
    explicit KdMFCC();
    ~KdMFCC();

    void Compute(float *signal, BtFrameBuf *out);
    KdWindow win;

private:
    void ComputeLifter();
    void computeDCTMatrix();
    void computePower(float *wav);

    float lifter_coeff[BT_FEAT_SIZE];
    float power_spec[BT_FFT_SIZE/2 + 1]; // power spectrum
    float dct_matrix[BT_MFCC_BIN][BT_MFCC_BIN];
    KdMelBanks* mel_banks;
    KdFFT *fft;

//    float *mel_energies[BT_MFCC_BIN];
    kaldi::Vector<float> mel_energies;
    float cepstral_lifter = 22.0;  // Scaling factor on cepstral
};
#endif // KD_MFCC_H
