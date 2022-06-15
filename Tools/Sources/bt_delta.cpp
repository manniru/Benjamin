#include "bt_delta.h"
#include <QDebug>

KdDelta::KdDelta(BtCFB *feat)
{
    feature   = feat;
    min_frame = 0;
    calcCoeffs();
}

void KdDelta::calcCoeffs()
{
    coeffs[0].resize(1);
    coeffs[0][0] = 1.0;  // trivial WINDOW for 0th order delta

    // calc scales
    float normalizer = sumof2N2(KD_DELTA_WINDOW);
    for( int i=0; i<BT_DELTA_ORDER ; i++)
    {
        int l_size = coeffs[i].length(); //last size
        int max_j = 2*KD_DELTA_WINDOW+1;
        initZeroCoeff(i+1, l_size + 2*KD_DELTA_WINDOW); // init with zero

        for( int j=0 ; j<max_j ; j++ )
        {
            for( int k=0 ; k<l_size ; k++ )
            {
                coeffs[i+1][j+k] += (j-KD_DELTA_WINDOW) * coeffs[i][k];
            }
        }

        int len2 = coeffs[i+1].length(); //last size
        for( int j=0 ; j<len2 ; j++ )
        {
            coeffs[i+1][j] *= 1.0 / normalizer;
        }
    }
}

// Get sum of 2*(1^2+2^2+...+n^2)
int KdDelta::sumof2N2(int n)
{
    return (n*(n+1)*(2*n+1))/3;
}

// normalize feat
void KdDelta::Process(uint frame, int max_frame)
{
    BtFrameBuf *buf = feature->get(frame);
    resetDelta(buf);

    for( int i=0 ; i<BT_DELTA_ORDER ; i++ )
    {
        int len = coeffs[i+1].length()-1;
        for( int j=-len/2; j<=len/2; j++ )
        {
            int i_frame = frame + j;
            if( i_frame<min_frame )
            {
                i_frame = min_frame;
            }
            else if( i_frame>max_frame )
            {
                i_frame = max_frame;
            }

            double *i_data = feature->get(i_frame)->delta;
            double *o_data = &(buf->delta[(i+1)*BT_FEAT_SIZE]);
            applyCoeff(i_data, coeffs[i+1][j+len/2], o_data);
        }
    }
}

void KdDelta::applyCoeff(double *i_data, double coeff, double *o_data)
{
    if( coeff==0 )
    {
        return;
    }

    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        o_data[i] += i_data[i] * coeff;
    }
}

// set delta to zero
void KdDelta::resetDelta(BtFrameBuf *buf)
{
    for( int i=0 ; i<BT_DELTA_ORDER ; i++ )
    {
        for( int j=0 ; j<BT_FEAT_SIZE ; j++ )
        {
            buf->delta[(i+1)*BT_FEAT_SIZE+j] = 0;
        }
    }
}

// set delta to zero
void KdDelta::initZeroCoeff(int i, int len)
{
    coeffs[i].resize(len); // init with zero
    for( int j=0 ; j<len ; j++ )
    {
        coeffs[i][j] = 0;
    }
}

