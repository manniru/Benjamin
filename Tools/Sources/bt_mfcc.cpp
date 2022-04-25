#include "bt_mfcc.h"
#include <QDebug>

BtMFCC::BtMFCC()
{
    computeDCTMatrix();

    ComputeLifter();

    fft = new KdFFT(win.fftSize());
    mel_banks = new BtMelBanks;
}

BtMFCC::~BtMFCC()
{
    delete mel_banks;
    delete fft;
}

void BtMFCC::Compute(float *signal, BtFrameBuf *out)
{
    fft->Compute(signal);
    computePower(signal);

    mel_banks->Compute(power_spec, mel_energies);

    for( int i=0 ; i<BT_MFCC_BIN ; i++ )
    {
        if( mel_energies[i]==0 )
        {
            qDebug() << "zero mel_energies";
            mel_energies[i] = std::numeric_limits<float>::epsilon();
        }
        mel_energies[i] = std::log(mel_energies[i]);
    }

    // feature = dct_matrix_ * mel_energies
    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        out->ceps[i] = 0;
        for( int j=0 ; j<BT_MFCC_BIN ; j++ )
        {
            out->ceps[i] += mel_energies[j]*dct_matrix[i][j];
        }
        out->ceps[i] *= lifter_coeff[i];
        out->cmvn[i] = out->ceps[i]; // cmvn val adds later
        out->have_cmvn = false;
    }
}

void BtMFCC::ComputeLifter()
{
    float Q = cepstral_lifter;
    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        lifter_coeff[i] = 1.0 + 0.5 * Q * sin(M_PI*i/Q);
    }
}

void BtMFCC::computePower(float *wav)
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

void BtMFCC::computeDCTMatrix()
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
