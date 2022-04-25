#ifndef BT_FFT_H
#define BT_FFT_H

#include <QDebug>
#include "bt_config.h"
#include "kd_token.h"

#include <vector>

// SplitRadixComplexFft
class KdSrcFFT
{
public:
    // N is the number of complex points (must be a power of two, or this
    // will crash).  Note that the constructor does some work so it's best to
    // initialize the object once and do the computation many times.
    KdSrcFFT(int N);

    // Does the FFT computation, given pointers to the float and
    // imaginary parts.  If "forward", do the forward FFT; else
    // do the inverse FFT (without the 1/N factor).
    // xr and xi are pointers to zero-based arrays of size N,
    // containing the float and imaginary parts
    // respectively.
    void Compute(float *xr, float *xi, bool forward);

    // This version of Compute is const; it operates on an array of size N*2
    // containing [ r0 im0 r1 im1 ... ], but it uses the argument "temp_buffer" as
    // temporary storage instead of a class-member variable.  It will allocate it if
    // needed.
    void Compute(float *x, bool forward, std::vector<float> *temp_buffer);

    ~KdSrcFFT();

protected:
    // temp_buffer_ is allocated only if someone calls Compute with only one float*
    // argument and we need a temporary buffer while creating interleaved data.
    std::vector<float> temp_buffer_;
private:
    void ComputeTables();
    void ComputeRecursive(float *xr, float *xi, int logn);
    void BitReversePermute(float *x, int logn);

    int N_;
    int logn_;  // log(N)

    int *brseed_; // see paper below
    // Evans, "An improved digit-reversal permutation algorithm ...",
    // IEEE Trans. ASSP, Aug. 1987.
    float **tab_;       // Tables of butterfly coefficients.
};

// will fail unless N>=4 and N is a power of 2.
class KdFFT: private KdSrcFFT
{
public:
    KdFFT(int N);

    /// output is a sequence of complex numbers of length N/2 with (float, im)
    /// format,  i.e. [float0, float_{N/2}, float1, im1, float2, im2,
    /// float3, im3, ...].
    void Compute(float *data);

private:
    int fft_size;
};

#endif // BT_FFT_H
