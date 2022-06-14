#include "bt_mfcc.h"
#include <QDebug>

BtMFCC::BtMFCC(int ft_size, int mfcc_size)
{
    feat_size = ft_size;
    bin_size  = mfcc_size;
    computeDCTMatrix();

    fft = new KdFFT(win.fftSize());
    mel_banks = new BtMelBanks(bin_size);
    mel_energies = (float *)malloc(sizeof(float)*bin_size);
    ComputeLifter();
}

BtMFCC::~BtMFCC()
{
    delete lifter_coeff;
    delete mel_energies;
    delete mel_banks;
    delete fft;

    for( int i=0 ; i<bin_size ; i++ )
    {
        delete dct_matrix[i];
    }
    delete dct_matrix;
}

void BtMFCC::Compute(float *signal, BtFrameBuf *out)
{
    computePower(signal);

    mel_banks->Compute(power_spec, mel_energies);
//    spec_min = 999;

    for( int i=0 ; i<bin_size ; i++ )
    {
        if( mel_energies[i]==0 )
        {
            qDebug() << "zero mel_energies" << feat_size;
            mel_energies[i] = std::numeric_limits<float>::epsilon();
        }
        mel_energies[i] = std::log(mel_energies[i]);
        if( mel_energies[i]<spec_min )
        {
            spec_min = mel_energies[i];
        }
        if( mel_energies[i]<11 )
        {
            spec_min = 11;
        }
    }
//    qDebug() << "min mel_energies" << spec_min;

    // feature = dct_matrix_ * mel_energies
    double *out_buf;
    if( feat_size>BT_FEAT_SIZE )
    {
        out_buf = out->enn;
    }
    else
    {
        out_buf = out->ceps;
    }

    for( int i=0 ; i<feat_size ; i++ )
    {
        out_buf[i] = 0;
        for( int j=0 ; j<bin_size ; j++ )
        {
            out_buf[i] += mel_energies[j]*dct_matrix[i][j];
        }
        out_buf[i] *= lifter_coeff[i];
        if( feat_size==BT_FEAT_SIZE )
        {
            out->cmvn[i] = out->ceps[i]; // cmvn val adds later
            out->have_cmvn = false;
        }
    }
}

void BtMFCC::ComputeENN(float *signal, BtFrameBuf *out)
{
    computePower(signal);

    mel_banks->Compute(power_spec, mel_energies);

    for( int i=0 ; i<feat_size ; i++ )
    {
        if( mel_energies[i]==0 )
        {
            qDebug() << "zero mel_energies" << feat_size;
            mel_energies[i] = std::numeric_limits<float>::epsilon();
        }
        out->enn[i] = std::log(mel_energies[i]);
    }
}

void BtMFCC::ComputeLifter()
{
    lifter_coeff = (float *)malloc(sizeof(float)*feat_size);

    float Q = cepstral_lifter;
    for( int i=0 ; i<feat_size ; i++ )
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
    dct_matrix = (float **)malloc(sizeof(float *)*bin_size);
    for( int i=0 ; i<bin_size ; i++ )
    {
        dct_matrix[i] = (float *)malloc(sizeof(float)*bin_size);;
    }

    float normalizer = std::sqrt(1.0/bin_size);
    for( int j=0 ; j<bin_size ; j++ )
    {
        dct_matrix[0][j] = normalizer;
    }
    normalizer = std::sqrt(2.0/bin_size);  // normalizer for other
    // elements.
    for( int k=1 ; k<bin_size ; k++ )
    {
        for( int n=0 ; n<bin_size ; n++ )
        {
            dct_matrix[k][n] = normalizer
                * std::cos( M_PI/bin_size * (n + 0.5) * k );
        }
    }
}
