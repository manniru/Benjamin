#ifndef BT_MELBANK_H
#define BT_MELBANK_H

#include "util/kaldi-io.h"
#include "base/kaldi-math.h"
#include "bt_window.h"
#include <QVector>

// must be >= BT_FEAT_SIZE
#define BT_MFCC_BIN 23 // 23 for 16khz, 15 for 8khz

class KdMelBanks
{
public:
    KdMelBanks();

    float InverseMelScale(float mel_freq);
    float MelScale(float freq);

    void Compute(float *power_spec,
                 kaldi::VectorBase<float> *out);
    float dotProduct(int bin_id,
                      int offset, float *v2);

    float freq_offset[BT_MFCC_BIN];
    QVector<float> bins_[BT_MFCC_BIN];
private:
    float low_freq = 20;  // lower frequency cutoff
};

#endif // KD_LATTICE_DECODER_H
