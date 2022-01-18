#include "kd_mfcc.h"
#include <QDebug>

using namespace kaldi;

void KdMFCC::Compute(float signal_raw_log_energy,
                           float vtln_warp,
                           VectorBase<float> *signal_frame,
                           VectorBase<float> *feature)
{
    KALDI_ASSERT(signal_frame->Dim() == frame_opts.PaddedWindowSize() &&
                 feature->Dim() == this->Dim());

    const MelBanks &mel_banks = *(GetMelBanks(vtln_warp));

    if (opts.use_energy && !opts.raw_energy)
        signal_raw_log_energy = Log(std::max<float>(VecVec(*signal_frame, *signal_frame),
                                                        std::numeric_limits<float>::epsilon()));

    if (srfft_ != NULL)  // Compute FFT using the split-radix algorithm.
        srfft_->Compute(signal_frame->Data(), true);
    else  // An alternative algorithm that works for non-powers-of-two.
        RealFft(signal_frame, true);

    // Convert the FFT into a power spectrum.
    ComputePowerSpectrum(signal_frame);
    SubVector<float> power_spectrum(*signal_frame, 0,
                                        signal_frame->Dim() / 2 + 1);

    mel_banks.Compute(power_spectrum, &mel_energies_);

    // avoid log of zero (which should be prevented anyway by dithering).
    mel_energies_.ApplyFloor(std::numeric_limits<float>::epsilon());
    mel_energies_.ApplyLog();  // take the log.

    feature->SetZero();  // in case there were NaNs.
    // feature = dct_matrix_ * mel_energies [which now have log]
    feature->AddMatVec(1.0, dct_matrix_, kNoTrans, mel_energies_, 0.0);

    if (opts.cepstral_lifter != 0.0)
        feature->MulElements(lifter_coeffs_);

    if (opts.use_energy) {
        if (opts.energy_floor > 0.0 && signal_raw_log_energy < log_energy_floor_)
            signal_raw_log_energy = log_energy_floor_;
        (*feature)(0) = signal_raw_log_energy;
    }
}

KdMFCC::KdMFCC()
{
    int num_bins = 23;
    opts.mel_opts = new MelBanksOptions(num_bins);
    mel_energies_.Resize(num_bins);
    srfft_ = NULL;
    if (opts.num_ceps > num_bins)
        KALDI_ERR << "num-ceps cannot be larger than num-mel-bins."
                  << " It should be smaller or equal. You provided num-ceps: "
                  << opts.num_ceps << "  and num-mel-bins: "
                  << num_bins;

    Matrix<float> dct_matrix(num_bins, num_bins);
    ComputeDctMatrix(&dct_matrix);
    // Note that we include zeroth dct in either case.  If using the
    // energy we replace this with the energy.  This means a different
    // ordering of features than HTK.
    SubMatrix<float> dct_rows(dct_matrix, 0, opts.num_ceps, 0, num_bins);
    dct_matrix_.Resize(opts.num_ceps, num_bins);
    dct_matrix_.CopyFromMat(dct_rows);  // subset of rows.
    if( opts.cepstral_lifter!=0.0 )
    {
        lifter_coeffs_.Resize(opts.num_ceps);
        ComputeLifterCoeffs(opts.cepstral_lifter, &lifter_coeffs_);
    }
    if( opts.energy_floor>0.0 )
    {
        log_energy_floor_ = Log(opts.energy_floor);
    }

    int padded_window_size = frame_opts.PaddedWindowSize();
    if ((padded_window_size & (padded_window_size-1)) == 0)  // Is a power of two...
        srfft_ = new SplitRadixRealFft<float>(padded_window_size);

    // We'll definitely need the filterbanks info for VTLN warping factor 1.0.
    // [note: this call caches it.]
    GetMelBanks(1.0);
}

KdMFCC::~KdMFCC()
{
    for (std::map<float, MelBanks*>::iterator iter = mel_banks_.begin();
         iter != mel_banks_.end();
         ++iter)
        delete iter->second;
    delete srfft_;
}

MelBanks *KdMFCC::GetMelBanks(float vtln_warp)
{
    MelBanks *this_mel_banks = NULL;
    std::map<float, MelBanks*>::iterator iter = mel_banks_.find(vtln_warp);
    if( iter==mel_banks_.end() )
    {
        this_mel_banks = new MelBanks(*(opts.mel_opts),
                                      frame_opts, vtln_warp);
        mel_banks_[vtln_warp] = this_mel_banks;
    }
    else
    {
        this_mel_banks = iter->second;
    }
    return this_mel_banks;
}

int KdMFCC::Dim()
{
    return opts.num_ceps;
}

bool KdMFCC::NeedRawLogEnergy()
{
    return opts.use_energy && opts.raw_energy;
}
