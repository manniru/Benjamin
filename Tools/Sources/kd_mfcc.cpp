#include "kd_mfcc.h"
#include <QDebug>

using namespace kaldi;

KdMFCC::KdMFCC()
{
    mel_energies.Resize(BT_MFCC_BIN);

    Matrix<float> dct_matrix(BT_MFCC_BIN, BT_MFCC_BIN);
    ComputeDctMatrix(&dct_matrix);
    // Note that we include zeroth dct in either case.  If using the
    // energy we replace this with the energy.  This means a different
    // ordering of features than HTK.
    SubMatrix<float> dct_rows(dct_matrix, 0, BT_FEAT_SIZE, 0, BT_MFCC_BIN);
    dct_matrix_.Resize(BT_FEAT_SIZE, BT_MFCC_BIN);
    dct_matrix_.CopyFromMat(dct_rows);  // subset of rows.

    ComputeLifter();

    fft = new KdFFT(win.fftSize());
    mel_banks_ = new KdMelBanks(BT_MFCC_BIN);
}

KdMFCC::~KdMFCC()
{
    if( mel_banks_ )
        delete mel_banks_;
    delete fft;
}

void KdMFCC::Compute(float *signal,
                     VectorBase<float> *feature)
{
    fft->Compute(signal);
    computePower(signal);

    mel_banks_->Compute(power_spec, &mel_energies);

    // avoid log of zero (which should be prevented anyway by dithering).
    mel_energies.ApplyFloor(std::numeric_limits<float>::epsilon());
    mel_energies.ApplyLog();  // take the log.

    feature->SetZero();  // in case there were NaNs.
    // feature = dct_matrix_ * mel_energies [which now have log]
    feature->AddMatVec(1.0, dct_matrix_, kNoTrans, mel_energies, 0.0);


//    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
//    {
//        for( int j=0 ; j)
//        (*feature)(i) *= lifter_coeff[i];
//    }

    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        (*feature)(i) *= lifter_coeff[i];
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
