#ifndef KD_MFCC_H
#define KD_MFCC_H

#include <QString>
#include "kd_window.h"
#include "kd_melbank.h"
#include "kd_fft.h"

class KdMFCC
{
public:
    explicit KdMFCC();
    ~KdMFCC();

    int Dim();

    void Compute(float *signal,
                 kaldi::VectorBase<float> *feature);
    KdWindow win;

protected:
    KdMelBanks *GetMelBanks();
    void ComputeLifterCoeffs(kaldi::VectorBase<float> *coeffs);
    void ComputePowerSpectrum(float *wav, kaldi::VectorBase<float> *power);

    kaldi::Vector<float> lifter_coeffs_;
    kaldi::Matrix<float> dct_matrix_;  // matrix we left-multiply by to perform DCT.
    KdMelBanks* mel_banks_;
    KdFFT *fft;

    // note: mel_energies_ is specific to the frame we're processing, it's
    // just a temporary workspace.
    kaldi::Vector<float> mel_energies_;
    // 23 for 16khz-sampled data,
    // for 8khz, 15 may be better.
    int num_ceps = 13;  // num cepstral bin
    float cepstral_lifter = 22.0;  // Scaling factor on cepstra
    int num_bins = 23; // mel bin
};
#endif // KD_MFCC_H
