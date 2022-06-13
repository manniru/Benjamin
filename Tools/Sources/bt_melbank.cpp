#include "bt_melbank.h"
#include <math.h>
#include <QDebug>

BtMelBanks::BtMelBanks(int count)
{
    bin_count = count;

    freq_offset = (float *)malloc(sizeof(float)*count);
    bins.resize(count);

    float nyquist = 0.5 * BT_REC_RATE;
    float high_freq = nyquist;
    // fft-bin width [think of it as Nyquist-freq / half-window-length]

    float mel_low_freq = MelScale(low_freq);
    float mel_high_freq = MelScale(high_freq);
    calcBins(mel_low_freq, mel_high_freq);
}

BtMelBanks::~BtMelBanks()
{
    free(freq_offset);
}

void BtMelBanks::Compute(float *power_spec, float *out)
{
    for( int i = 0; i<bin_count ; i++ )
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
    int len = bins[bin_id].length();
    float sum = 0;
    for( int i=0 ; i<len ; i++ )
    {
        sum += bins[bin_id][i] * v2[offset+i];
    }

    return sum;
}

void BtMelBanks::calcBins(float mel_lfreq, float mel_hfreq)
{
    int   num_fft_bins  = BT_FFT_SIZE / 2;
    float fft_bin_width = BT_REC_RATE / BT_FFT_SIZE;

    // divide by num_bins+1 in next line because of end-effects where the bins
    // spread out to the sides.
    float mel_freq_delta = (mel_hfreq - mel_lfreq) / (bin_count+1);

    for( int i = 0; i<bin_count ; i++ )
    {
        float left_mel   = mel_lfreq + i * mel_freq_delta;
        float center_mel = mel_lfreq + (i + 1) * mel_freq_delta;
        float right_mel  = mel_lfreq + (i + 2) * mel_freq_delta;

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
                bins[i].push_back(weight);
                if( freq_offset[i]==-1 )
                {
                    freq_offset[i] = j;
                }
            }
        }
    }
}
