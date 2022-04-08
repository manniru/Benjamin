#ifndef KD_MELBANK_H
#define KD_MELBANK_H

#include "util/kaldi-io.h"
#include "base/kaldi-math.h"
#include "kd_window.h"

class KdMelBanks
{
public:
    KdMelBanks(int bin_count);

    float InverseMelScale(float mel_freq);
    float MelScale(float freq);

    void Compute(kaldi::VectorBase<float> &fft_energies,
                 kaldi::VectorBase<float> *mel_energies_out);

    kaldi::Vector<float> center_freqs_;
    std::vector<std::pair<int, kaldi::Vector<float> > > bins_;
private:
    int num_bins = 25;  // must be > 3
    float low_freq = 20;  // e.g. 20; lower frequency cutoff
    float high_freq = 0;  // an upper frequency cutoff;
                          // 0 -> no cutoff,
                          // negative -> added to the Nyquist frequency
};

#endif // KD_LATTICE_DECODER_H
