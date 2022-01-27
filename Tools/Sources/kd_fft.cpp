#include "kd_fft.h"

using namespace kaldi;

// forward = true instead of inverse
void KdFFT::Compute(float *data)
{
    MatrixIndexT N = N_, N2 = N/2;

    SplitRadixComplexFft<float>::Compute(data, true, &temp_buffer_);

    float rootN_re, rootN_im;  // exp(-2pi/N)
    int forward_sign = -1;
    ComplexImExp(static_cast<float>(M_2PI/N *forward_sign), &rootN_re, &rootN_im);
    float kN_re = -forward_sign, kN_im = 0.0;  // exp(-2pik/N), forward; exp(-2pik/N), backward
    // kN starts out as 1.0 for forward algorithm but -1.0 for backward.
    for (MatrixIndexT k = 1; 2*k <= N2; k++)
    {
        ComplexMul(rootN_re, rootN_im, &kN_re, &kN_im);

        float Ck_re, Ck_im, Dk_re, Dk_im;
        // C_k = 1/2 (B_k + B_{N/2 - k}^*) :
        Ck_re = 0.5 * (data[2*k] + data[N - 2*k]);
        Ck_im = 0.5 * (data[2*k + 1] - data[N - 2*k + 1]);
        // re(D_k)= 1/2 (im(B_k) + im(B_{N/2-k})):
        Dk_re = 0.5 * (data[2*k + 1] + data[N - 2*k + 1]);
        // im(D_k) = -1/2 (re(B_k) - re(B_{N/2-k}))
        Dk_im =-0.5 * (data[2*k] - data[N - 2*k]);
        // A_k = C_k + 1^(k/N) D_k:
        data[2*k] = Ck_re;  // A_k <-- C_k
        data[2*k+1] = Ck_im;
        // now A_k += D_k 1^(k/N)
        ComplexAddProduct(Dk_re, Dk_im, kN_re, kN_im, &(data[2*k]), &(data[2*k+1]));

        MatrixIndexT kdash = N2 - k;
        if (kdash != k) {
            // Next we handle the index k' = N/2 - k.  This is necessary
            // to do now, to avoid invalidating data that we will later need.
            // The quantities C_{k'} and D_{k'} are just the conjugates of C_k
            // and D_k, so the equations are simple modifications of the above,
            // replacing Ck_im and Dk_im with their negatives.
            data[2*kdash] = Ck_re;  // A_k' <-- C_k'
            data[2*kdash+1] = -Ck_im;
            // now A_k' += D_k' 1^(k'/N)
            // We use 1^(k'/N) = 1^((N/2 - k) / N) = 1^(1/2) 1^(-k/N) = -1 * (1^(k/N))^*
            // so it's the same as 1^(k/N) but with the float part negated.
            ComplexAddProduct(Dk_re, -Dk_im, -kN_re, kN_im, &(data[2*kdash]), &(data[2*kdash+1]));
        }
    }
    // Now handle k = 0.
    // In simple terms: after the complex fft, data[0] becomes the sum of float
    // parts input[0], input[2]... and data[1] becomes the sum of imaginary
    // pats input[1], input[3]...
    // "zeroth" [A_0] is just the sum of input[0]+input[1]+input[2]..
    // and "n2th" [A_{N/2}] is input[0]-input[1]+input[2]... .
    float zeroth = data[0] + data[1];
    float   n2th = data[0] - data[1];
    data[0] = zeroth;
    data[1] = n2th;
}
