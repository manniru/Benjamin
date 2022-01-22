#ifndef KD_MFCC_H
#define KD_MFCC_H

#include "feat/mel-computations.h"
#include "feat/feature-window.h"
#include "feat/feature-functions.h"
#include <QString>
#include "kd_window.h"
#include "kd_melbank.h"

// 23 for 16khz-sampled data,
// for 8khz, 15 may be better.
struct KdMfccOptions
{
    kaldi::MelBanksOptions *mel_opts;
    int32 num_ceps = 13;  // num cepstral coeffs, counting zero.
    bool use_energy = false;  // use energy; else C0
    float energy_floor = 0;  // set to a value like 1.0 or 0.1 if
    // you disable dithering.
    bool raw_energy = true;  // If true, compute energy before preemphasis and windowing
    float cepstral_lifter = 22.0;  // Scaling factor on cepstra for HTK compatibility.
    // if 0.0, no liftering is done.
};

class KdMFCC
{
public:
    explicit KdMFCC();
    ~KdMFCC();

    int Dim();
    bool NeedRawLogEnergy();

    void Compute(float signal_raw_log_energy,
                 kaldi::VectorBase<float> *signal_frame,
                 kaldi::VectorBase<float> *feature);
    KdMfccOptions opts;
    KdWindow frame_opts;

protected:
    KdMelBanks *GetMelBanks();

    kaldi::Vector<float> lifter_coeffs_;
    kaldi::Matrix<float> dct_matrix_;  // matrix we left-multiply by to perform DCT.
    float log_energy_floor_;
    std::map<float, KdMelBanks*> mel_banks_;  // float is VTLN coefficient.
    kaldi::SplitRadixRealFft<float> *srfft_;

    // note: mel_energies_ is specific to the frame we're processing, it's
    // just a temporary workspace.
    kaldi::Vector<float> mel_energies_;
};
#endif // KD_MFCC_H
