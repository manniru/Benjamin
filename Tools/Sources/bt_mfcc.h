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
    explicit BtMFCC(int ft_size=BT_FEAT_SIZE,
                    int mfcc_size=BT_MFCC_BIN);
    ~BtMFCC();

    void Compute(float *signal, BtFrameBuf *out);
    void ComputeENN(float *signal, BtFrameBuf *out);
    BtWindow win;
    KdFFT *fft; //fft would be called from outside

private:
    void ComputeLifter();
    void computeDCTMatrix();
    void computePower(float *wav);

    float *lifter_coeff;
    float power_spec[BT_FFT_SIZE/2 + 1]; // power spectrum
    float **dct_matrix;
    BtMelBanks* mel_banks;

    float *mel_energies;
    float cepstral_lifter = 22.0;  // Scaling factor on cepstral

    int feat_size;
    int bin_size;
};
#endif // BT_MFCC_H
