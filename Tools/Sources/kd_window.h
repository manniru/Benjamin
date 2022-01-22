#ifndef KD_WINDOW_H
#define KD_WINDOW_H

#include "util/kaldi-io.h"
#include "base/kaldi-math.h"
#include "bt_config.h"

int kd_RoundP2(int n);

class KdWindow // FrameExtractionOptions + FeatureWindowFunction
{
public:
    KdWindow();
    int WindowShift();
    int WindowSize();
    int PaddedWindowSize();
    int NumFrames(int num_samples);
    void ExtractWindow(int sample_offset,
                                 const kaldi::VectorBase<float> &wave,
                                 int f, kaldi::Vector<float> *window,
                                 float *log_energy_pre_window);
    void ProcessWindow(kaldi::VectorBase<float> *win,
                       float *log_energy_pre_window = NULL);
    void Preemphasize(kaldi::VectorBase<float> *waveform, float preemph_coeff);
    void Dither(kaldi::VectorBase<float> *waveform, float dither_value);


    int FirstSampleOfFrame(int32 frame);

    float samp_freq = BT_REC_RATE;
    float frame_shift_ms = 10.0;
    float frame_length_ms = 25.0;
    float dither = 1.0;  // Amount of dithering, 0.0 means no dither.
    float preemph_coeff = 0.97;  // Preemphasis coefficient.
    bool remove_dc_offset = true;
    std::string window_type = "povey";  // "hamming", "rectangular", "povey", "hanning", "sine", "blackman"
    bool round_to_power_of_two = true;
    float blackman_coeff = 0.42;
    bool snip_edges = true;
    bool allow_downsample = false;
    bool allow_upsample = false;
    int max_feature_vectors = -1;

private:
    kaldi::Vector<float> window;
};

#endif // KD_WINDOW_H
