#include "bt_melbank.h"
#include <math.h>
#include <QDebug>

BtMelBanks::BtMelBanks()
{
    float sample_freq = BT_REC_RATE;

    int num_fft_bins = BT_FFT_SIZE / 2;
    float nyquist = 0.5 * sample_freq;
    float high_freq = nyquist;

    float fft_bin_width = sample_freq / BT_FFT_SIZE;
    // fft-bin width [think of it as Nyquist-freq / half-window-length]

    float mel_low_freq = MelScale(low_freq);
    float mel_high_freq = MelScale(high_freq);

    // divide by num_bins+1 in next line because of end-effects where the bins
    // spread out to the sides.
    float mel_freq_delta = (mel_high_freq - mel_low_freq) / (BT_MFCC_BIN+1);

    for( int i = 0; i<BT_MFCC_BIN ; i++ )
    {
        float left_mel   = mel_low_freq + i * mel_freq_delta;
        float center_mel = mel_low_freq + (i + 1) * mel_freq_delta;
        float right_mel  = mel_low_freq + (i + 2) * mel_freq_delta;

        freq_offset[i] = -1;
        for( int j=0; j<num_fft_bins ; j++)
        {
            float freq = (fft_bin_width * j);  // Center frequency of this fft
            // bin.
            float mel = MelScale(freq);
            if( mel>left_mel && mel<right_mel )
            {
                float weight;
                if( mel <= center_mel)
                {
                    weight = (mel - left_mel) / (center_mel - left_mel);
                }
                else
                {
                    weight = (right_mel-mel) / (right_mel-center_mel);
                }
                bins_[i].push_back(weight);
                if( freq_offset[i]==-1 )
                {
                    freq_offset[i] = j;
                }
            }
        }
    }
}

void BtMelBanks::Compute(float *power_spec, float *out)
{
    for( int i = 0; i<BT_MFCC_BIN ; i++ )
    {
        int offset = freq_offset[i];
        out[i] = dotProduct(i, offset, power_spec);
    }
}

float BtMelBanks::MelScale(float freq)
{
    return 1127.0f * log(1.0f + freq / 700.0f);
}

float BtMelBanks::InverseMelScale(float mel_freq)
{
    return 700.0f * (expf (mel_freq / 1127.0f) - 1.0f);
}

// return vector dot product
float BtMelBanks::dotProduct(int bin_id, int offset, float *v2)
{
    int len = bins_[bin_id].length();
    float sum = 0;
    for( int i=0 ; i<len ; i++ )
    {
        sum += bins_[bin_id][i] * v2[offset+i];
    }

    return sum;
}
