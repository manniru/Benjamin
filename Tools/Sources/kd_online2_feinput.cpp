#include "kd_online2_feinput.h"

using namespace kaldi;

KdOnline2FeInput::KdOnline2FeInput()
{
    std::string mfcc_config = KAL_NATO_DIR"exp/tri1_online/conf/mfcc.conf";
    std::string cmvn_config = KAL_NATO_DIR"exp/tri1_online/conf/online_cmvn.conf";
    std::string  global_cmvn_stats_rxfilename = KAL_NATO_DIR"exp/tri1_online/global_cmvn.stats";

    ReadConfigFromFile(mfcc_config, &mfcc_opts);
    ReadConfigFromFile(cmvn_config, &cmvn_opts);
    ReadKaldiObject(global_cmvn_stats_rxfilename,
                    &global_cmvn_stats_);
    Init();
}

OnlineFeatureInterface* KdOnline2FeInput::AdaptedFeature() const
{
    if (fmllr_)
        return fmllr_;
    else
        return delta;
}

// Init() is to be called from the constructor; it assumes the pointer
// members are all uninitialized but config_ and lda_mat_ are
// initialized.
void KdOnline2FeInput::Init()
{
    base_feature_ = new OnlineMfcc(mfcc_opts);
    KALDI_ASSERT(global_cmvn_stats_.NumRows() != 0);
    Matrix<double> global_cmvn_stats_dbl(global_cmvn_stats_);
    OnlineCmvnState initial_state(global_cmvn_stats_dbl);
    cmvn = new OnlineCmvn(cmvn_opts, initial_state, base_feature_);

    delta = new OnlineDeltaFeature(delta_opts, cmvn);
    fmllr_ = NULL;  // This will be set up if the user calls SetTransform().
}

void KdOnline2FeInput::SetTransform(MatrixBase<float> &transform)
{
    if (fmllr_ != NULL)
    {
        delete fmllr_;
        fmllr_ = NULL;
    }

    if( transform.NumRows()!=0 )
    {
        fmllr_ = new OnlineTransform(transform, delta);
    }
}


void KdOnline2FeInput::FreezeCmvn()
{
    cmvn->Freeze(cmvn->NumFramesReady() - 1);
}

int32 KdOnline2FeInput::Dim() const
{
    return AdaptedFeature()->Dim();
}
bool KdOnline2FeInput::IsLastFrame(int32 frame) const
{
    return AdaptedFeature()->IsLastFrame(frame);
}
int32 KdOnline2FeInput::NumFramesReady() const
{
    return AdaptedFeature()->NumFramesReady();
}

void KdOnline2FeInput::GetFrame(int32 frame,
                                VectorBase<float> *feat)
{
    AdaptedFeature()->GetFrame(frame, feat);
}

KdOnline2FeInput::~KdOnline2FeInput()
{
  // Note: the delete command only deletes pointers that are non-NULL.  Not all
  // of the pointers below will be non-NULL.
  delete fmllr_;
  delete delta;
  // Guard against double deleting the cmvn_ ptr
  delete cmvn;
  delete base_feature_;
}

void KdOnline2FeInput::AcceptWaveform(float sampling_rate,
                                      VectorBase<float> &waveform)
{
    base_feature_->AcceptWaveform(sampling_rate, waveform);
}

void KdOnline2FeInput::InputFinished()
{
  base_feature_->InputFinished();
}

float KdOnline2FeInput::FrameShiftInSeconds() const
{
    return mfcc_opts.frame_opts.frame_shift_ms / 1000.0f;
}
