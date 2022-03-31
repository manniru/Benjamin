#include "kd_delta.h"
#include <QDebug>

using namespace kaldi;

KdDelta::KdDelta()
{
    scales.resize(KD_DELTA_ORDER+1);
    scales[0].Resize(1);
    scales[0](0) = 1.0;  // trivial WINDOW for 0th order delta

    // calc scales
    float normalizer = sumof2N2(KD_DELTA_WINDOW);
    for( int i=0; i<KD_DELTA_ORDER ; i++)
    {
        int l_size = scales[i].Dim(); //last size
        int max_j = 2*KD_DELTA_WINDOW+1;
        scales[i+1].Resize(l_size + 2*KD_DELTA_WINDOW); // init with zero

        for( int j=0 ; j<max_j ; j++ )
        {
            for (int k=0 ; k<l_size ; k++ )
            {
                scales[i+1](j+k) += (j-KD_DELTA_WINDOW) * scales[i](k);
            }
        }

        scales[i+1].Scale(1.0 / normalizer);
    }
}

// Get sum of 2*(1^2+2^2+...+n^2)
int KdDelta::sumof2N2(int n)
{
    return (n*(n+1)*(2*n+1))/3;
}

void KdDelta::Process(MatrixBase<float> &input_feats,
                      int frame,
                      VectorBase<float> *output_frame)
{
    int num_frames = input_feats.NumRows();
    int feat_dim = input_feats.NumCols();
    KALDI_ASSERT(output_frame->Dim()==(feat_dim*(KD_DELTA_ORDER+1)));
    output_frame->SetZero();

    for( int i=0 ; i<(KD_DELTA_ORDER+1) ; i++ )
    {
        int len = scales[i].Dim();
        SubVector<float> output(*output_frame, i*feat_dim, feat_dim);
        for( int j=0; j<len; j++ )
        {
            // if asked to read
            int offset_frame = frame + j - (len-1)/2;
            if( offset_frame<0 )
            {
                offset_frame = 0;
            }
            else if( offset_frame >= num_frames)
            {
                offset_frame = num_frames - 1;
            }

            float coeff = scales[i](j);
            if( coeff!=0 )
            {
                output.AddVec(coeff, input_feats.Row(offset_frame));
            }
        }
    }
}
