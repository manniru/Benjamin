#ifndef KD_FFT_H
#define KD_FFT_H

#include <QDebug>
#include "bt_config.h".
#include "kd_token2.h"

#include <vector>
#include <map>

#include "feat/feature-functions.h"

class KdFFT: private kaldi::SplitRadixComplexFft<float>
{
public:
    KdFFT(int N):  // will fail unless N>=4 and N is a power of 2.
        kaldi::SplitRadixComplexFft<float> (N/2), N_(N) { }

    /// output is a sequence of complex numbers of length N/2 with (float, im) format,
    /// i.e. [float0, float_{N/2}, float1, im1, float2, im2, float3, im3, ...].
    void Compute(float *data);

private:
    int N_;
};

#endif // KD_FFT_H
