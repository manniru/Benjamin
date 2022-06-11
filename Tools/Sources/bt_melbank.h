#ifndef BT_MELBANK_H
#define BT_MELBANK_H

#include "bt_window.h"
#include <QVector>

// must be >= BT_FEAT_SIZE
#define BT_MFCC_BIN 23 // 23 for 16khz, 15 for 8khz

class BtMelBanks
{
public:
    BtMelBanks(int count);
    ~BtMelBanks();

    float InverseMelScale(float mel_freq);
    float MelScale(float freq);

    void Compute(float *power_spec,
                 float *out);

    float *freq_offset;
    QVector<QVector<float>> bins;
private:
    float dotProduct(int bin_id, int offset, float *v2);
    void  calcBins(float mel_lfreq, float mel_hfreq);

    int   bin_count = BT_MFCC_BIN;
    float low_freq = 20;  // lower frequency cutoff
};

#endif // KD_LATTICE_DECODER_H
