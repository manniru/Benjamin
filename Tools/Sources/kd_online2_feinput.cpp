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
    mfcc = new MfccComputer(mfcc_opts);
    o_features = new RecyclingVector(mfcc_opts.frame_opts.max_feature_vectors);
    KALDI_ASSERT(global_cmvn_stats_.NumRows() != 0);
    Matrix<double> global_cmvn_stats_dbl(global_cmvn_stats_);
    OnlineCmvnState initial_state(global_cmvn_stats_dbl);
    cmvn = new OnlineCmvn(cmvn_opts, initial_state, mfcc);

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
    feat->CopyFromVec(*(o_features->At(frame)));
}

KdOnline2FeInput::~KdOnline2FeInput()
{
  // Note: the delete command only deletes pointers that are non-NULL.  Not all
  // of the pointers below will be non-NULL.
  delete fmllr_;
  delete delta;
  // Guard against double deleting the cmvn_ ptr
  delete cmvn;
  delete mfcc;
}

void KdOnline2FeInput::AcceptWaveform(float sampling_rate,
                                      VectorBase<float> &waveform)
{
    if( waveform.Dim()==0 )
      return;

    Vector<BaseFloat> appended_wave;

    VectorBase<float> *waveform;

    appended_wave.Resize(waveform_remainder_.Dim() + waveform->Dim());
    if (waveform_remainder_.Dim() != 0)
      appended_wave.Range(0, waveform_remainder_.Dim())
          .CopyFromVec(waveform_remainder_);
    appended_wave.Range(waveform_remainder_.Dim(), waveform->Dim())
        .CopyFromVec(*waveform);
    waveform_remainder_.Swap(&appended_wave);
    ComputeFeatures();
}

void KdOnline2FeInput::ComputeFeatures()
{
  const FrameExtractionOptions &frame_opts = mfcc.GetFrameOptions();
  int64 num_samples_total = waveform_offset_ + waveform_remainder_.Dim();
  int32 num_frames_old = features_.Size(),
      num_frames_new = NumFrames(num_samples_total, frame_opts,
                                 input_finished_);
  KALDI_ASSERT(num_frames_new >= num_frames_old);

  Vector<BaseFloat> window;
  bool need_raw_log_energy = mfcc.NeedRawLogEnergy();
  for (int32 frame = num_frames_old; frame < num_frames_new; frame++) {
    BaseFloat raw_log_energy = 0.0;
    ExtractWindow(waveform_offset_, waveform_remainder_, frame,
                  frame_opts, window_function_, &window,
                  need_raw_log_energy ? &raw_log_energy : NULL);
    Vector<BaseFloat> *this_feature = new Vector<BaseFloat>(mfcc.Dim(),
                                                            kUndefined);
    // note: this online feature-extraction code does not support VTLN.
    BaseFloat vtln_warp = 1.0;
    mfcc.Compute(raw_log_energy, vtln_warp, &window, this_feature);
    features_.PushBack(this_feature);
  }
  // OK, we will now discard any portion of the signal that will not be
  // necessary to compute frames in the future.
  int64 first_sample_of_next_frame = FirstSampleOfFrame(num_frames_new,
                                                        frame_opts);
  int32 samples_to_discard = first_sample_of_next_frame - waveform_offset_;
  if (samples_to_discard > 0) {
    // discard the leftmost part of the waveform that we no longer need.
    int32 new_num_samples = waveform_remainder_.Dim() - samples_to_discard;
    if (new_num_samples <= 0) {
      // odd, but we'll try to handle it.
      waveform_offset_ += waveform_remainder_.Dim();
      waveform_remainder_.Resize(0);
    } else {
      Vector<BaseFloat> new_remainder(new_num_samples);
      new_remainder.CopyFromVec(waveform_remainder_.Range(samples_to_discard,
                                                          new_num_samples));
      waveform_offset_ += samples_to_discard;
      waveform_remainder_.Swap(&new_remainder);
    }
  }
}

void KdOnline2FeInput::InputFinished()
{
    ComputeFeatures();
}

float KdOnline2FeInput::FrameShiftInSeconds() const
{
    return mfcc_opts.frame_opts.frame_shift_ms / 1000.0f;
}
