#include "kd_melbank.h"

using namespace kaldi;
KdMelBanks::KdMelBanks(MelBanksOptions &opts,
                   KdWindow &frame_opts, float vtln_warp_factor):
    htk_mode_(opts.htk_mode) {
    int32 num_bins = opts.num_bins;
    if (num_bins < 3) KALDI_ERR << "Must have at least 3 mel bins";
    float sample_freq = frame_opts.samp_freq;
    int32 window_length_padded = frame_opts.PaddedWindowSize();
    KALDI_ASSERT(window_length_padded % 2 == 0);
    int32 num_fft_bins = window_length_padded / 2;
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

    float vtln_low = opts.vtln_low,
            vtln_high = opts.vtln_high;
    if (vtln_high < 0.0) {
        vtln_high += nyquist;
    }

    if (vtln_warp_factor != 1.0 &&
            (vtln_low < 0.0 || vtln_low <= low_freq
             || vtln_low >= high_freq
             || vtln_high <= 0.0 || vtln_high >= high_freq
             || vtln_high <= vtln_low))
        KALDI_ERR << "Bad values in options: vtln-low " << vtln_low
                  << " and vtln-high " << vtln_high << ", versus "
                  << "low-freq " << low_freq << " and high-freq "
                  << high_freq;

    bins_.resize(num_bins);
    center_freqs_.Resize(num_bins);

    for (int32 bin = 0; bin < num_bins; bin++)
    {
        float left_mel = mel_low_freq + bin * mel_freq_delta,
                center_mel = mel_low_freq + (bin + 1) * mel_freq_delta,
                right_mel = mel_low_freq + (bin + 2) * mel_freq_delta;

        center_freqs_(bin) = InverseMelScale(center_mel);
        // this_bin will be a vector of coefficients that is only
        // nonzero where this mel bin is active.
        Vector<float> this_bin(num_fft_bins);
        int32 first_index = -1, last_index = -1;
        for (int32 i = 0; i < num_fft_bins; i++) {
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
        int32 size = last_index + 1 - first_index;
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
    int32 num_bins = bins_.size();
    KALDI_ASSERT(mel_energies_out->Dim() == num_bins);

    for (int32 i = 0; i < num_bins; i++) {
        int32 offset = bins_[i].first;
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
        for (int32 i = 0; i < num_bins; i++)
            fprintf(stderr, " %f", (*mel_energies_out)(i));
        fprintf(stderr, "\n");
    }
}

void ComputeLifterCoeffs(float Q, VectorBase<float> *coeffs) {
    // Compute liftering coefficients (scaling on cepstral coeffs)
    // coeffs are numbered slightly differently from HTK: the zeroth
    // index is C0, which is not affected.
    for (int32 i = 0; i < coeffs->Dim(); i++)
        (*coeffs)(i) = 1.0 + 0.5 * Q * sin (M_PI * i / Q);
}


// Durbin's recursion - converts autocorrelation coefficients to the LPC
// pTmp - temporal place [n]
// pAC - autocorrelation coefficients [n + 1]
// pLP - linear prediction coefficients [n] (predicted_sn = sum_1^P{a[i-1] * s[n-i]}})
//       F(z) = 1 / (1 - A(z)), 1 is not stored in the demoninator
float Durbin(int n, const float *pAC, float *pLP, float *pTmp) {
    float ki;                // reflection coefficient
    int i;
    int j;

    float E = pAC[0];

    for (i = 0; i < n; i++) {
        // next reflection coefficient
        ki = pAC[i + 1];
        for (j = 0; j < i; j++)
            ki += pLP[j] * pAC[i - j];
        ki = ki / E;

        // new error
        float c = 1 - ki * ki;
        if (c < 1.0e-5) // remove NaNs for constan signal
            c = 1.0e-5;
        E *= c;

        // new LP coefficients
        pTmp[i] = -ki;
        for (j = 0; j < i; j++)
            pTmp[j] = pLP[j] - ki * pLP[i - j - 1];

        for (j = 0; j <= i; j++)
            pLP[j] = pTmp[j];
    }

    return E;
}
