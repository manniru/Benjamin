#include "kd_melbank.h"

using namespace kaldi;
KdMelBanks::KdMelBanks(MelBanksOptions &opts,
                   KdWindow &frame_opts):
    htk_mode_(opts.htk_mode) {
    int num_bins = opts.num_bins;
    if (num_bins < 3) KALDI_ERR << "Must have at least 3 mel bins";
    float sample_freq = frame_opts.samp_freq;
    int window_length_padded = frame_opts.PaddedWindowSize();
    KALDI_ASSERT(window_length_padded % 2 == 0);
    int num_fft_bins = window_length_padded / 2;
    float nyquist = 0.5 * sample_freq;

    float low_freq = opts.low_freq, high_freq;
    if (opts.high_freq > 0.0)
        high_freq = opts.high_freq;
    else
        high_freq = nyquist + opts.high_freq;

    if (low_freq < 0.0 || low_freq >= nyquist
            || high_freq <= 0.0 || high_freq > nyquist
            || high_freq <= low_freq)
        KALDI_ERR << "Bad values in options: low-freq " << low_freq
                  << " and high-freq " << high_freq << " vs. nyquist "
                  << nyquist;

    float fft_bin_width = sample_freq / window_length_padded;
    // fft-bin width [think of it as Nyquist-freq / half-window-length]

    float mel_low_freq = MelScale(low_freq);
    float mel_high_freq = MelScale(high_freq);

    debug_ = opts.debug_mel;

    // divide by num_bins+1 in next line because of end-effects where the bins
    // spread out to the sides.
    float mel_freq_delta = (mel_high_freq - mel_low_freq) / (num_bins+1);

    bins_.resize(num_bins);
    center_freqs_.Resize(num_bins);

    for (int bin = 0; bin < num_bins; bin++)
    {
        float left_mel = mel_low_freq + bin * mel_freq_delta;
        float center_mel = mel_low_freq + (bin + 1) * mel_freq_delta;
        float right_mel = mel_low_freq + (bin + 2) * mel_freq_delta;

        center_freqs_(bin) = InverseMelScale(center_mel);
        // this_bin will be a vector of coefficients that is only
        // nonzero where this mel bin is active.
        Vector<float> this_bin(num_fft_bins);
        int first_index = -1, last_index = -1;
        for( int i = 0; i < num_fft_bins; i++) {
            float freq = (fft_bin_width * i);  // Center frequency of this fft
            // bin.
            float mel = MelScale(freq);
            if (mel > left_mel && mel < right_mel) {
                float weight;
                if (mel <= center_mel)
                    weight = (mel - left_mel) / (center_mel - left_mel);
                else
                    weight = (right_mel-mel) / (right_mel-center_mel);
                this_bin(i) = weight;
                if (first_index == -1)
                    first_index = i;
                last_index = i;
            }
        }
        KALDI_ASSERT(first_index != -1 && last_index >= first_index
                && "You may have set --num-mel-bins too large.");

        bins_[bin].first = first_index;
        int size = last_index + 1 - first_index;
        bins_[bin].second.Resize(size);
        bins_[bin].second.CopyFromVec(this_bin.Range(first_index, size));

        // Replicate a bug in HTK, for testing purposes.
        if (opts.htk_mode && bin == 0 && mel_low_freq != 0.0)
            bins_[bin].second(0) = 0.0;

    }
    if (debug_) {
        for (size_t i = 0; i < bins_.size(); i++) {
            KALDI_LOG << "bin " << i << ", offset = " << bins_[i].first
                      << ", vec = " << bins_[i].second;
        }
    }
}

// "power_spectrum" contains fft energies.
void KdMelBanks::Compute(const VectorBase<float> &power_spectrum,
                       VectorBase<float> *mel_energies_out) const {
    int num_bins = bins_.size();
    KALDI_ASSERT(mel_energies_out->Dim() == num_bins);

    for (int i = 0; i < num_bins; i++) {
        int offset = bins_[i].first;
        const Vector<float> &v(bins_[i].second);
        float energy = VecVec(v, power_spectrum.Range(offset, v.Dim()));
        // HTK-like flooring- for testing purposes (we prefer dither)
        if (htk_mode_ && energy < 1.0) energy = 1.0;
        (*mel_energies_out)(i) = energy;

        // The following assert was added due to a problem with OpenBlas that
        // we had at one point (it was a bug in that library).  Just to detect
        // it early.
        KALDI_ASSERT(!KALDI_ISNAN((*mel_energies_out)(i)));
    }

    if (debug_) {
        fprintf(stderr, "MEL BANKS:\n");
        for (int i = 0; i < num_bins; i++)
            fprintf(stderr, " %f", (*mel_energies_out)(i));
        fprintf(stderr, "\n");
    }
}

float KdMelBanks::MelScale(float freq)
{
    return 1127.0f * logf (1.0f + freq / 700.0f);
}

float KdMelBanks::InverseMelScale(float mel_freq)
{
    return 700.0f * (expf (mel_freq / 1127.0f) - 1.0f);
}
