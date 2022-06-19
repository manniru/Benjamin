#ifndef BT_WINDOW_H
#define BT_WINDOW_H

#include "config.h"
#include <QString>

// round to power of 2
int kd_RoundP2(int n);

class BtWindow // FrameExtractionOptions + FeatureWindowFunction
{
public:
    BtWindow();
    int frameShift();
    int frameLen();
    int fftSize();
    int frameCount(int num_samples);
    void ProcessWindow(float *win);
    void Preemphasize(float *wav);
    void removeDC(float *wav);

    int FirstSampleOfFrame(int frame);

private:
    float window[BT_FFT_SIZE];

    float samp_freq = BT_REC_RATE;
    float frame_shift_ms = 10.0;
    float frame_length_ms = 25.0;
    float dither = 1.0;  // Amount of dithering, 0.0 means no dither.
    float preemph_coeff = 0.97;  // must be between 0 and 1
    // "hamming", "rectangular", "povey", "hanning", "sine", "blackman"
    QString window_type = "povey";
    float blackman_coeff = 0.42;
};

#endif // BT_WINDOW_H
