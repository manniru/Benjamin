#include "kd_window.h"
#include <QDebug>

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

int KdWindow::fftSize()
{
    return kd_RoundP2(WindowSize());
}

KdWindow::KdWindow()
{
    int frame_length = WindowSize();
    KALDI_ASSERT(frame_length > 0);
    window.Resize(frame_length);
    double a = M_2PI / (frame_length-1);
    for (int i = 0; i < frame_length; i++)
    {
        double i_fl = static_cast<double>(i);
        if( window_type == "hanning")
        {
            window(i) = 0.5  - 0.5*cos(a * i_fl);
        }
        else if( window_type == "sine")
        {
            // when you are checking ws wikipedia, please
            // note that 0.5 * a = M_PI/(frame_length-1)
            window(i) = sin(0.5 * a * i_fl);
        }
        else if( window_type == "hamming")
        {
            window(i) = 0.54 - 0.46*cos(a * i_fl);
        }
        else if( window_type == "povey")
        {  // like hamming but goes to zero at edges.
            window(i) = pow(0.5 - 0.5*cos(a * i_fl), 0.85);
        }
        else if( window_type == "rectangular")
        {
            window(i) = 1.0;
        }
        else if( window_type == "blackman")
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

int KdWindow::frameCount(int num_samples)
{
    int frame_length = WindowSize();
    if( num_samples<frame_length )
    {
        return 0;
    }
    else
    {
        int frame_shift = WindowShift();
        return (1 + ((num_samples - frame_length) / frame_shift));
    }
}

void KdWindow::extract(int offset, VectorBase<float> &wave,
                       Vector<float> *win)
{
    int frame_length = WindowSize();
    int fft_size = fftSize();

    if( win->Dim()!=fft_size )
    {
        win->Resize(fft_size, kUndefined);
    }

    int wave_end = offset + frame_length;
    if( wave_end>wave.Dim() )
    {
        qDebug() << "FFFFFFFFFFFFFFFFFFFFFFFFF";
    }
    win->Range(0, frame_length).CopyFromVec(
                wave.Range(offset, frame_length));

    // add zero padded data
    if( fft_size>frame_length )
        win->Range(frame_length, fft_size - frame_length).SetZero();

    QString buf;
    for( int i=0 ; i<win->Dim() ; i++ )
    {
        buf += QString::number((*win)(i));
        buf += " ";
    }

    qDebug() << frame_num << frame_length
             << win->Dim() << buf;
    frame_num++;

    SubVector<float> frame(*win, 0, frame_length);

    ProcessWindow(&frame);
}


void KdWindow::ProcessWindow(VectorBase<float> *win)
{
    int frame_length = WindowSize();
    KALDI_ASSERT(win->Dim() == frame_length);

    Dither(win, dither);

    // remove_dc_offset
    win->Add(-win->Sum()/frame_length);

    Preemphasize(win, preemph_coeff);

    win->MulElements(window);
}

void KdWindow::Dither(VectorBase<float> *waveform, float dither_value)
{
    if( dither_value == 0.0)
    {
        return;
    }
    int dim = waveform->Dim();
    float *data = waveform->Data();
    RandomState rstate;
    for( int i=0 ; i<dim ; i++ )
    {
        data[i] += RandGauss(&rstate) * dither_value;
    }
}


void KdWindow::Preemphasize(VectorBase<float> *waveform, float preemph_coeff)
{
    if( preemph_coeff == 0.0) return;
    KALDI_ASSERT(preemph_coeff >= 0.0 && preemph_coeff <= 1.0);
    for (int i = waveform->Dim()-1 ; i > 0 ; i-- )
    {
        (*waveform)(i) -= preemph_coeff * (*waveform)(i-1);
    }
    (*waveform)(0) -= preemph_coeff * (*waveform)(0);
}

