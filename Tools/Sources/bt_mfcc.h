#ifndef BT_MFCC_H
#define BT_MFCC_H

#include <QString>
#include "bt_window.h"
#include "bt_melbank.h"
#include "bt_fft.h"
#include "bt_cfb.h"

class BtMFCC
{
public:
    explicit BtMFCC();
    ~BtMFCC();

    void Compute(float *signal, BtFrameBuf *out);
    BtWindow win;

private:
    void ComputeLifter();
    void computeDCTMatrix();
    void computePower(float *wav);

    float lifter_coeff[BT_FEAT_SIZE];
    float power_spec[BT_FFT_SIZE/2 + 1]; // power spectrum
    float dct_matrix[BT_MFCC_BIN][BT_MFCC_BIN];
    BtMelBanks* mel_banks;
    KdFFT *fft;

//    float *mel_energies[BT_MFCC_BIN];
    float mel_energies[BT_MFCC_BIN];
    float cepstral_lifter = 22.0;  // Scaling factor on cepstral
};
#endif // BT_MFCC_H
