#ifndef KD_MELBANK_H
#define KD_MELBANK_H

#include "kd_window.h"
#include "feat/mel-computations.h"

class KdMelBanks
{
public:

    float InverseMelScale(float mel_freq);
    float MelScale(float freq);

    KdMelBanks(kaldi::MelBanksOptions &opts,
               KdWindow &frame_opts);

    /// Compute Mel energies (note: not log enerties).
    /// At input, "fft_energies" contains the FFT energies (not log).
    void Compute(const kaldi::VectorBase<float> &fft_energies,
                 kaldi::VectorBase<float> *mel_energies_out) const;

    // center frequencies of bins
    kaldi::Vector<float> center_freqs_;

    // the "bins_" vector is a vector, one for each bin, of a pair(the vector of weights).
    std::vector<std::pair<int, kaldi::Vector<float> > > bins_;
private:
    bool debug_;
    bool htk_mode_;
};

#endif // KD_LATTICE_DECODER_H
