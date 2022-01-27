#ifndef KD_MELBANK_H
#define KD_MELBANK_H

#include "kd_window.h"
#include "feat/mel-computations.h"


struct MelBanksOptions {
};

class KdMelBanks
{
public:

    float InverseMelScale(float mel_freq);
    float MelScale(float freq);

    KdMelBanks(int bin_count,
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
    int num_bins = 25;  // e.g. 25; number of triangular bins
    float low_freq = 20;  // e.g. 20; lower frequency cutoff
    float high_freq = 0;  // an upper frequency cutoff; 0 -> no cutoff, negative
    // ->added to the Nyquist frequency to get the cutoff.
};

#endif // KD_LATTICE_DECODER_H
