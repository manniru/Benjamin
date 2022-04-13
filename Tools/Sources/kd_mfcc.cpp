#include "kd_mfcc.h"
#include <QDebug>

using namespace kaldi;

KdMFCC::KdMFCC()
{
    mel_energies.Resize(BT_MFCC_BIN);

//    dct_matrix.Resize(BT_MFCC_BIN, BT_MFCC_BIN);
//    ComputeDctMatrix(&dct_matrix);
    computeDCTMatrix();

    ComputeLifter();

    fft = new KdFFT(win.fftSize());
    mel_banks = new KdMelBanks(BT_MFCC_BIN);
}

KdMFCC::~KdMFCC()
{
    delete mel_banks;
    delete fft;
}

void KdMFCC::Compute(float *signal, BtFrameBuf *out)
{
    fft->Compute(signal);
    computePower(signal);

    mel_banks->Compute(power_spec, &mel_energies);

    // avoid log of zero (which should be prevented anyway by dithering).
    mel_energies.ApplyFloor(std::numeric_limits<float>::epsilon());
    mel_energies.ApplyLog();  // take the log.

    // feature = dct_matrix_ * mel_energies
    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        out->ceps[i] = 0;
        for( int j=0 ; j<BT_MFCC_BIN ; j++ )
        {
            out->ceps[i] += mel_energies(j)*dct_matrix[i][j];
        }
        out->ceps[i] *= lifter_coeff[i];
        out->cmvn[i] = out->ceps[i]; // cmvn val adds later
        out->have_cmvn = false;
    }
}

void KdMFCC::ComputeLifter()
{
    float Q = cepstral_lifter;
    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        lifter_coeff[i] = 1.0 + 0.5 * Q * sin(M_PI*i/Q);
    }
}

void KdMFCC::computePower(float *wav)
{
    int len = BT_FFT_SIZE/2;
    power_spec[0]   = wav[0] * wav[0];
    power_spec[len] = wav[1] * wav[1];

    for( int i=1 ; i<len ; i++ )
    {
        float im   = wav[i*2 + 1];
        float real = wav[i*2];
        power_spec[i] = real*real + im*im;
    }
}

void KdMFCC::computeDCTMatrix()
{
    float normalizer = std::sqrt(1.0/BT_MFCC_BIN);
    for( int j=0 ; j<BT_MFCC_BIN ; j++ )
    {
        dct_matrix[0][j] = normalizer;
    }
    normalizer = std::sqrt(2.0/BT_MFCC_BIN);  // normalizer for other
    // elements.
    for( int k=1 ; k<BT_MFCC_BIN ; k++ )
    {
        for (int n=0 ; n<BT_MFCC_BIN ; n++ )
        {
            dct_matrix[k][n] = normalizer
                * std::cos( M_PI/BT_MFCC_BIN * (n + 0.5) * k );
        }
    }
}
