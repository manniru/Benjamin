#ifndef KD_WINDOW_H
#define KD_WINDOW_H

#include "util/kaldi-io.h"
#include "base/kaldi-math.h"
#include "bt_config.h"

int kd_RoundP2(int n);

struct KdWinOpt // FrameExtractionOptions
{
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

    int WindowShift()
    {
        return samp_freq * 0.001 * frame_shift_ms;
    }
    int WindowSize()
    {
        return samp_freq * 0.001 * frame_length_ms;
    }
    int PaddedWindowSize()
    {
        return kd_RoundP2(WindowSize());
    }
};

#endif // KD_WINDOW_H
