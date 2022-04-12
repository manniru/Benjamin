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

    void Compute(float *signal,
                 kaldi::VectorBase<float> *feature);
    KdWindow win;

private:
    void ComputeLifter();
    void computePower(float *wav);

    float lifter_coeff[BT_FEAT_SIZE];
    float power_spec[BT_FFT_SIZE/2 + 1]; // power spectrum
    kaldi::Matrix<float> dct_matrix_;  // matrix we left-multiply by to perform DCT.
    KdMelBanks* mel_banks_;
    KdFFT *fft;

    // note: mel_energies_ is specific to the frame we're processing, it's
    // just a temporary workspace.
//    float *mel_energies[BT_MFCC_BIN];
    kaldi::Vector<float> mel_energies;
    float cepstral_lifter = 22.0;  // Scaling factor on cepstral
};
#endif // KD_MFCC_H
