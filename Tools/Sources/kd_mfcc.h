#ifndef KD_MFCC_H
#define KD_MFCC_H

#include "feat/mel-computations.h"
#include "feat/feature-window.h"
#include "feat/feature-functions.h"
#include <QString>
#include "kd_window.h"

/**
 @param [in] signal_raw_log_energy The log-energy of the frame of the signal
     prior to windowing and pre-emphasis, or
     log(numeric_limits<float>::min()), whichever is greater.  Must be
     ignored by this function if this class returns false from
     this->NeedsRawLogEnergy().
 @param [in] vtln_warp  The VTLN warping factor that the user wants
     to be applied when computing features for this utterance.  Will
     normally be 1.0, meaning no warping is to be done.  The value will
     be ignored for feature types that don't support VLTN, such as
     spectrogram features.
 @param [in] signal_frame  One frame of the signal,
   as extracted using the function ExtractWindow() using the options
   returned by this->GetFrameOptions().  The function will use the
   vector as a workspace, which is why it's a non-const pointer.
 @param [out] feature  Pointer to a vector of size this->Dim(), to which
     the computed feature will be written.
*/
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
    bool htk_compat = false;  // if true, put energy/C0 last and introduce a factor of
    // sqrt(2) on C0 to be the same as HTK.
};

class KdMFCC
{
public:
    explicit KdMFCC();
    ~KdMFCC();

    int Dim();
    bool NeedRawLogEnergy();

    void Compute(float signal_raw_log_energy,
                 float vtln_warp,
                 kaldi::VectorBase<float> *signal_frame,
                 kaldi::VectorBase<float> *feature);
    KdMfccOptions opts;
    KdWinOpt frame_opts;

protected:
    kaldi::MelBanks *GetMelBanks(float vtln_warp);

    kaldi::Vector<float> lifter_coeffs_;
    kaldi::Matrix<float> dct_matrix_;  // matrix we left-multiply by to perform DCT.
    float log_energy_floor_;
    std::map<float, kaldi::MelBanks*> mel_banks_;  // BaseFloat is VTLN coefficient.
    kaldi::SplitRadixRealFft<float> *srfft_;

    // note: mel_energies_ is specific to the frame we're processing, it's
    // just a temporary workspace.
    kaldi::Vector<float> mel_energies_;
};
#endif // KD_MFCC_H
