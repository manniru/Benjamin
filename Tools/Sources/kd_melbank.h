#ifndef KD_MELBANK_H
#define KD_MELBANK_H

#include "kd_window.h"
#include "feat/mel-computations.cc"

class KdMelBanks
{
public:

    float InverseMelScale(float mel_freq)
    {
        return 700.0f * (expf (mel_freq / 1127.0f) - 1.0f);
    }

    float MelScale(float freq)
    {
        return 1127.0f * logf (1.0f + freq / 700.0f);
    }

    KdMelBanks(kaldi::MelBanksOptions &opts,
               KdWindow &frame_opts,
               float vtln_warp_factor);

    /// Compute Mel energies (note: not log enerties).
    /// At input, "fft_energies" contains the FFT energies (not log).
    void Compute(const kaldi::VectorBase<float> &fft_energies,
                 kaldi::VectorBase<float> *mel_energies_out) const;

//    int32 NumBins() const { return bins_.size(); }

//    // returns vector of central freq of each bin; needed by plp code.
//    kaldi::Vector<float> &GetCenterFreqs() const { return center_freqs_; }

//    std::vector<std::pair<int32, kaldi::Vector<float> > >& GetBins() {
//        return bins_;
//    }

    // center frequencies of bins, numbered from 0 ... num_bins-1.
    // Needed by GetCenterFreqs().
    kaldi::Vector<float> center_freqs_;

    // the "bins_" vector is a vector, one for each bin, of a pair:
    // (the first nonzero fft-bin), (the vector of weights).
    std::vector<std::pair<int32, kaldi::Vector<float> > > bins_;
private:
    bool debug_;
    bool htk_mode_;
};

#endif // KD_LATTICE_DECODER_H
