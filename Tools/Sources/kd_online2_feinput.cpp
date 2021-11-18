#include "kd_online2_feinput.h"

using namespace kaldi

KdOnline2FeInput::KdOnline2FeInput()
{
    OnlineFeaturePipelineCommandLineConfig config;
    feature_type = "mfcc";
    std::string mfcc_config = KAL_NATO_DIR"exp/tri1_online/conf/mfcc.conf";
    std::string cmvn_config = KAL_NATO_DIR"exp/tri1_online/conf/online_cmvn.conf";
    global_cmvn_stats_rxfilename = KAL_NATO_DIR"exp/tri1_online/global_cmvn.stats";
    add_deltas = true;

    ReadConfigFromFile(mfcc_config, &mfcc_opts);
    ReadConfigFromFile(cmvn_config, &cmvn_opts);
    ReadKaldiObject(config.global_cmvn_stats_rxfilename,
                    &global_cmvn_stats_);
    Init();
}

KdOnline2FeInput* KdOnline2FeInput::UnadaptedFeature()
{
  return splice_or_delta_;
}

KdOnline2FeInput* KdOnline2FeInput::AdaptedFeature()
{
  if (fmllr_)
      return fmllr_;
  else
    return UnadaptedFeature();
}

// Init() is to be called from the constructor; it assumes the pointer
// members are all uninitialized but config_ and lda_mat_ are
// initialized.
void KdOnline2FeInput::Init()
{
    base_feature_ = new OnlineMfcc(config_.mfcc_opts);
    KALDI_ASSERT(global_cmvn_stats_.NumRows() != 0);
    if (config_.add_pitch) {
      int32 global_dim = global_cmvn_stats_.NumCols() - 1;
      int32 dim = base_feature_->Dim();
      KALDI_ASSERT(global_dim >= dim);
      if (global_dim > dim) {
        Matrix<BaseFloat> last_col(global_cmvn_stats_.ColRange(global_dim, 1));
        global_cmvn_stats_.Resize(global_cmvn_stats_.NumRows(), dim + 1,
                                  kCopyData);
        global_cmvn_stats_.ColRange(dim, 1).CopyFromMat(last_col);
      }
    }
    Matrix<double> global_cmvn_stats_dbl(global_cmvn_stats_);
    OnlineCmvnState initial_state(global_cmvn_stats_dbl);
    cmvn_ = new OnlineCmvn(config_.cmvn_opts, initial_state, base_feature_);

    feature_ = cmvn_;

    splice_or_delta_ = new OnlineDeltaFeature(config_.delta_opts,
                                              feature_);

    fmllr_ = NULL;  // This will be set up if the user calls SetTransform().
}

void OnlineFeaturePipeline::SetTransform(MatrixBase<float> &transform)
{
  if (fmllr_ != NULL) {  // we already had a transform;  delete this
    // object.
    delete fmllr_;
    fmllr_ = NULL;
  }
  if (transform.NumRows() != 0) {
    OnlineFeatureInterface *feat = UnadaptedFeature();
    fmllr_ = new OnlineTransform(transform, feat);
  }
}


void OnlineFeaturePipeline::FreezeCmvn()
{
  cmvn_->Freeze(cmvn_->NumFramesReady() - 1);
}

int32 OnlineFeaturePipeline::Dim() const
{
  return AdaptedFeature()->Dim();
}
bool OnlineFeaturePipeline::IsLastFrame(int32 frame) const
{
  return AdaptedFeature()->IsLastFrame(frame);
}
int32 OnlineFeaturePipeline::NumFramesReady() const
{
  return AdaptedFeature()->NumFramesReady();
}

void KdOnline2FeInput::GetFrame(int32 frame,
                                VectorBase<BaseFloat> *feat)
{
  AdaptedFeature()->GetFrame(frame, feat);
}

KdOnline2FeInput::~OnlineFeaturePipeline()
{
  // Note: the delete command only deletes pointers that are non-NULL.  Not all
  // of the pointers below will be non-NULL.
  delete fmllr_;
  delete splice_or_delta_;
  // Guard against double deleting the cmvn_ ptr
  delete cmvn_;
  delete base_feature_;
}

void KdOnline2FeInput::AcceptWaveform(
    BaseFloat sampling_rate,
    const VectorBase<BaseFloat> &waveform) {
  base_feature_->AcceptWaveform(sampling_rate, waveform);
  if (pitch_)
    pitch_->AcceptWaveform(sampling_rate, waveform);
}

void KdOnline2FeInput::InputFinished() {
  base_feature_->InputFinished();
  if (pitch_)
    pitch_->InputFinished();
}

BaseFloat KdOnline2FeInput::FrameShiftInSeconds()
{
    return mfcc_opts.frame_opts.frame_shift_ms / 1000.0f;
}

void KdOnline2FeInput::GetAsMatrix(Matrix<BaseFloat> *feats) {
  if (pitch_) {
    feats->Resize(NumFramesReady(), pitch_feature_->Dim());
    for (int32 i = 0; i < NumFramesReady(); i++) {
      SubVector<BaseFloat> row(*feats, i);
      pitch_feature_->GetFrame(i, &row);
    }
  }
}

float KdOnline2FeInput::FrameShiftInSeconds()
{
    return config_.FrameShiftInSeconds();
}
