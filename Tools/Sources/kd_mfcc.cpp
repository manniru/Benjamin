#include "kd_mfcc.h"
#include <QDebug>

using namespace kaldi;

void KdMFCC::Compute(float *signal,
                     VectorBase<float> *feature)
{
    mel_banks_ = NULL;

    KdMelBanks &mel_banks = *(GetMelBanks());

    // Compute FFT using the split-radix algorithm.
    fft->Compute(signal);

    // Convert the FFT into a power spectrum.
    Vector<float> power_spectrum(BT_FFT_SIZE / 2 + 1);
    ComputePowerSpectrum(signal, &power_spectrum);

    mel_banks.Compute(power_spectrum, &mel_energies_);

    // avoid log of zero (which should be prevented anyway by dithering).
    mel_energies_.ApplyFloor(std::numeric_limits<float>::epsilon());
    mel_energies_.ApplyLog();  // take the log.

    feature->SetZero();  // in case there were NaNs.
    // feature = dct_matrix_ * mel_energies [which now have log]
    feature->AddMatVec(1.0, dct_matrix_, kNoTrans, mel_energies_, 0.0);

    feature->MulElements(lifter_coeffs_);
}

KdMFCC::KdMFCC()
{
    mel_energies_.Resize(num_bins);
    fft = NULL;
    mel_banks_ = NULL;
    if( num_ceps>num_bins )
        qDebug() << "num-ceps cannot be larger than num-mel-bins."
                 << num_ceps << "  and num-mel-bins: "
                 << num_bins;

    Matrix<float> dct_matrix(num_bins, num_bins);
    ComputeDctMatrix(&dct_matrix);
    // Note that we include zeroth dct in either case.  If using the
    // energy we replace this with the energy.  This means a different
    // ordering of features than HTK.
    SubMatrix<float> dct_rows(dct_matrix, 0, num_ceps, 0, num_bins);
    dct_matrix_.Resize(num_ceps, num_bins);
    dct_matrix_.CopyFromMat(dct_rows);  // subset of rows.

    lifter_coeffs_.Resize(num_ceps);
    ComputeLifterCoeffs(&lifter_coeffs_);

    fft = new KdFFT(win.fftSize());

    // We'll definitely need the filterbanks info for VTLN warping factor 1.0.
    // [note: this call caches it.]
    GetMelBanks();
}

KdMFCC::~KdMFCC()
{
    if( mel_banks_ )
        delete mel_banks_;
    delete fft;
}

KdMelBanks *KdMFCC::GetMelBanks()
{
    if( mel_banks_==NULL )
    {
        mel_banks_ = new KdMelBanks(num_bins);
    }
    return mel_banks_;
}

// Compute liftering coefficients
void KdMFCC::ComputeLifterCoeffs(VectorBase<float> *coeffs)
{
    float Q = cepstral_lifter;
    for( int i=0 ; i<coeffs->Dim() ; i++ )
    {
        (*coeffs)(i) = 1.0 + 0.5 * Q * sin(M_PI*i/Q);
    }
}

void KdMFCC::ComputePowerSpectrum(float *wav, VectorBase<float> *power)
{
    int half_dim = BT_FFT_SIZE/2;
    float first_energy = wav[0] * wav[0];
    float last_energy  = wav[1] * wav[1];  // handle this special case

    for (int i = 1; i < half_dim; i++)
    {
        float real = wav[i*2];
        float im = wav[i*2 + 1];
        (*power)(i) = real*real + im*im;
    }

    (*power)(0) = first_energy;
    (*power)(half_dim) = last_energy;  // Will actually never be used, and anyway
    // if the signal has been bandlimited sensibly this should be zero.
}

int KdMFCC::Dim()
{
    return num_ceps;
}
