#include "kd_melbank.h"

using namespace kaldi;
KdMelBanks::KdMelBanks(int bin_count)
{
    int num_bins = bin_count;

    float sample_freq = BT_REC_RATE;
    int window_length_padded = BT_FFT_SIZE;

    int num_fft_bins = window_length_padded / 2;
    float nyquist = 0.5 * sample_freq;
    high_freq = nyquist;

    float fft_bin_width = sample_freq / window_length_padded;
    // fft-bin width [think of it as Nyquist-freq / half-window-length]

    float mel_low_freq = MelScale(low_freq);
    float mel_high_freq = MelScale(high_freq);

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
        for( int i = 0; i < num_fft_bins; i++)
        {
            float freq = (fft_bin_width * i);  // Center frequency of this fft
            // bin.
            float mel = MelScale(freq);
            if( mel > left_mel && mel < right_mel)
            {
                float weight;
                if( mel <= center_mel)
                    weight = (mel - left_mel) / (center_mel - left_mel);
                else
                    weight = (right_mel-mel) / (right_mel-center_mel);
                this_bin(i) = weight;
                if( first_index==-1)
                    first_index = i;
                last_index = i;
            }
        }
        KALDI_ASSERT(first_index!=-1 && last_index >= first_index
                && "You may have set --num-mel-bins too large.");

        bins_[bin].first = first_index;
        int size = last_index + 1 - first_index;
        bins_[bin].second.Resize(size);
        bins_[bin].second.CopyFromVec(this_bin.Range(first_index, size));

    }
}

void KdMelBanks::Compute(float *power_spec,
                       VectorBase<float> *out)
{
    int num_bins = bins_.size();
    KALDI_ASSERT(out->Dim()==num_bins);

    for (int i = 0; i < num_bins; i++)
    {
        int offset = bins_[i].first;
        (*out)(i) = dotProduct(&(bins_[i].second), offset, power_spec);
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

// return vector dot product
float KdMelBanks::dotProduct(Vector<float> *v1,
                             int offset, float *v2)
{
    int len = v1->Dim();
    float sum = 0;
    for( int i=0 ; i<len ; i++ )
    {
        sum += (*v1)(i) * v2[offset+i];
    }

    return sum;
}
