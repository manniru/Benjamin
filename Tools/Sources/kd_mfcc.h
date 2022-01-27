#ifndef KD_MFCC_H
#define KD_MFCC_H

#include "feat/feature-functions.h"
#include <QString>
#include "kd_window.h"
#include "kd_melbank.h"

class KdMFCC
{
public:
    explicit KdMFCC();
    ~KdMFCC();

    int Dim();

    void Compute(kaldi::VectorBase<float> *signal_frame,
                 kaldi::VectorBase<float> *feature);
    KdWindow frame_opts;

protected:
    KdMelBanks *GetMelBanks();

    kaldi::Vector<float> lifter_coeffs_;
    kaldi::Matrix<float> dct_matrix_;  // matrix we left-multiply by to perform DCT.
    KdMelBanks* mel_banks_;
    kaldi::SplitRadixRealFft<float> *srfft_;

    // note: mel_energies_ is specific to the frame we're processing, it's
    // just a temporary workspace.
    kaldi::Vector<float> mel_energies_;
    // 23 for 16khz-sampled data,
    // for 8khz, 15 may be better.
    int num_ceps = 13;  // num cepstral coeffs
    float cepstral_lifter = 22.0;  // Scaling factor on cepstra for HTK compatibility.
    int num_bins = 23;
};
#endif // KD_MFCC_H
