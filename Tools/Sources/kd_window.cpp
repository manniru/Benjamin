#include "kd_window.h"

using namespace kaldi;

// Round up to nearest power of two
int kd_RoundP2(int n)
{
    KALDI_ASSERT(n > 0);
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n+1;
}

int KdWindow::WindowShift()
{
    return samp_freq * 0.001 * frame_shift_ms;
}

int KdWindow::WindowSize()
{
    return samp_freq * 0.001 * frame_length_ms;
}

int KdWindow::PaddedWindowSize()
{
    return kd_RoundP2(WindowSize());
}

KdWindow::KdWindow()
{
    int32 frame_length = WindowSize();
    KALDI_ASSERT(frame_length > 0);
    window.Resize(frame_length);
    double a = M_2PI / (frame_length-1);
    for (int32 i = 0; i < frame_length; i++)
    {
        double i_fl = static_cast<double>(i);
        if (window_type == "hanning")
        {
            window(i) = 0.5  - 0.5*cos(a * i_fl);
        }
        else if (window_type == "sine")
        {
            // when you are checking ws wikipedia, please
            // note that 0.5 * a = M_PI/(frame_length-1)
            window(i) = sin(0.5 * a * i_fl);
        }
        else if (window_type == "hamming")
        {
            window(i) = 0.54 - 0.46*cos(a * i_fl);
        }
        else if (window_type == "povey")
        {  // like hamming but goes to zero at edges.
            window(i) = pow(0.5 - 0.5*cos(a * i_fl), 0.85);
        }
        else if (window_type == "rectangular")
        {
            window(i) = 1.0;
        }
        else if (window_type == "blackman")
        {
            window(i) = blackman_coeff - 0.5*cos(a * i_fl) +
                    (0.5 - blackman_coeff) * cos(2 * a * i_fl);
        }
        else
        {
            KALDI_ERR << "Invalid window type " << window_type;
        }
    }
}

int KdWindow::NumFrames(int num_samples)
{
    int64 frame_shift = WindowShift();
    int64 frame_length = WindowSize();
    if( num_samples<frame_length )
    {
        return 0;
    }
    else
    {
        return (1 + ((num_samples - frame_length) / frame_shift));
    }
    // 'num_samples - frame_length' is how much room we have to shift the frame within the
    // waveform; 'frame_shift' is how much we shift it each time; and the ratio
    // is how many times we can shift it (integer arithmetic rounds down).
}

void KdWindow::ExtractWindow(int sample_offset,
                             const VectorBase<float> &wave,
                             int f, Vector<float> *win,
                             float *log_energy_pre_window)
{
    KALDI_ASSERT(sample_offset >= 0 && wave.Dim() != 0);
    int frame_length = WindowSize();
    int frame_length_padded = PaddedWindowSize();
    int num_samples = sample_offset + wave.Dim();
    int start_sample = FirstSampleOfFrame(f);
    int end_sample = start_sample + frame_length;

    KALDI_ASSERT(start_sample >= sample_offset &&
                 end_sample <= num_samples);

    if( win->Dim()!=frame_length_padded )
    {
        win->Resize(frame_length_padded, kUndefined);
    }

    // wave_start and wave_end are start and end indexes into 'wave', for the
    // piece of wave that we're trying to extract.
    int wave_start = start_sample - sample_offset;
    int wave_end = wave_start + frame_length;
    if (wave_start >= 0 && wave_end <= wave.Dim())
    {
        // the normal case-- no edge effects to consider.
        win->Range(0, frame_length).CopyFromVec(
                    wave.Range(wave_start, frame_length));
    }
    else
    {
        // Deal with any end effects by reflection, if needed.  This code will only
        // be reached for about two frames per utterance, so we don't concern
        // ourselves excessively with efficiency.
        int32 wave_dim = wave.Dim();
        for (int32 s = 0; s < frame_length; s++)
        {
            int32 s_in_wave = s + wave_start;
            while (s_in_wave < 0 || s_in_wave >= wave_dim)
            {
                // reflect around the beginning or end of the wave.
                // e.g. -1 -> 0, -2 -> 1.
                // dim -> dim - 1, dim + 1 -> dim - 2.
                // the code supports repeated reflections, although this
                // would only be needed in pathological cases.
                if (s_in_wave < 0) s_in_wave = - s_in_wave - 1;
                else s_in_wave = 2 * wave_dim - 1 - s_in_wave;
            }
            (*win)(s) = wave(s_in_wave);
        }
    }

    if (frame_length_padded > frame_length)
        win->Range(frame_length, frame_length_padded - frame_length).SetZero();

    SubVector<BaseFloat> frame(*win, 0, frame_length);

    ProcessWindow(&frame, log_energy_pre_window);
}

int KdWindow::FirstSampleOfFrame(int32 frame)
{
    return frame * WindowShift();
}


