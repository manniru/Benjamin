#include "bt_window.h"
#include <QDebug>
#include <math.h>

// Round up to nearest power of two
int kd_RoundP2(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n+1;
}

int BtWindow::frameShift()
{
    return samp_freq * 0.001 * frame_shift_ms;
}

int BtWindow::frameLen()
{
    return samp_freq * 0.001 * frame_length_ms;
}

int BtWindow::fftSize()
{
    return kd_RoundP2(frameLen());
}

BtWindow::BtWindow()
{
    int frame_length = frameLen();
    double a = 2*M_PI / (frame_length-1);
    for (int i = 0; i < frame_length; i++)
    {
        double i_fl = i;
        if( window_type=="hanning" )
        {
            window[i] = 0.5  - 0.5*cos(a * i_fl);
        }
        else if( window_type=="sine" )
        {
            // when you are checking ws wikipedia, please
            // note that 0.5 * a = M_PI/(frame_length-1)
            window[i] = sin(0.5 * a * i_fl);
        }
        else if( window_type=="hamming" )
        {
            window[i] = 0.54 - 0.46*cos(a * i_fl);
        }
        else if( window_type=="povey")
        {  // like hamming but goes to zero at edges.
            window[i] = pow(0.5 - 0.5*cos(a * i_fl), 0.85);
        }
        else if( window_type=="rectangular" )
        {
            window[i] = 1.0;
        }
        else if( window_type=="blackman" )
        {
            window[i] = blackman_coeff - 0.5*cos(a * i_fl) +
                    (0.5 - blackman_coeff) * cos(2 * a * i_fl);
        }
        else
        {
            qDebug() << "Invalid window type " << window_type;
        }
    }
}

int BtWindow::frameCount(int num_samples)
{
    int frame_length = frameLen();
    if( num_samples<frame_length )
    {
        return 0;
    }
    else
    {
        int frame_shift = frameShift();
        return (1 + ((num_samples - frame_length) / frame_shift));
    }
}

void BtWindow::ProcessWindow(float *win)
{
    removeDC(win);
    Preemphasize(win);

    int len = frameLen();
    for( int i=0 ; i<len ; i++ )
    {
        win[i] *= window[i];
    }
}

void BtWindow::removeDC(float *wav)
{
    int len = frameLen();
    double sum = 0;
    for( int i=0 ; i<len ; i++ )
    {
        sum += wav[i];
    }
    float dc_offset = sum/len;
    for( int i=0 ; i<len ; i++ )
    {
        wav[i] -= dc_offset;
    }
}

void BtWindow::Preemphasize(float *wav)
{
    int len = frameLen();
    for( int i=len-1 ; i>0 ; i-- )
    {
        wav[i] -= preemph_coeff * wav[i-1];
    }
    wav[0] -= preemph_coeff * wav[0];
}

