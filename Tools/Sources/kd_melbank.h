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

    void Compute(float *power_spec,
                 kaldi::VectorBase<float> *out);
    float dotProduct(kaldi::Vector<float> *v1,
                      int offset, float *v2);

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
