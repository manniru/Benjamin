#include "kd_online2_feinput.h"

using namespace kaldi;

KdOnline2FeInput::KdOnline2FeInput()
{
    waveform_offset = 0;
    std::string mfcc_config = KAL_NATO_DIR"exp/tri1_online/conf/mfcc.conf";
    std::string cmvn_config = KAL_NATO_DIR"exp/tri1_online/conf/online_cmvn.conf";
    std::string  global_cmvn_stats_rxfilename = KAL_NATO_DIR"exp/tri1_online/global_cmvn.stats";

    ReadConfigFromFile(mfcc_config, &mfcc_opts);
    ReadConfigFromFile(cmvn_config, &cmvn_opts);
    ReadKaldiObject(global_cmvn_stats_rxfilename,
                    &global_cmvn_stats_);
    Init();
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
//    OnlineCmvnState initial_state(global_cmvn_stats_dbl);
//    cmvn = new OnlineCmvn(cmvn_opts, initial_state, mfcc);

    delta_features = new DeltaFeatures(delta_opts);
}

void KdOnline2FeInput::FreezeCmvn()
{
    cmvn->Freeze(cmvn->NumFramesReady() - 1);
}

int32 KdOnline2FeInput::Dim() const
{
    int32 mfcc_dim = mfcc->Dim();
    return mfcc_dim * (1 + delta_opts.order);
}
bool KdOnline2FeInput::IsLastFrame(int32 frame) const
{
    return false;
}
int32 KdOnline2FeInput::NumFramesReady() const
{
    int32 num_frames = o_features->Size();
    // number of frames that is less to produce the output.
    int32 context = delta_opts.order * delta_opts.window;
    int32 ret     = num_frames - context;
    if( ret>0 )
    {
        return ret;
    }
    return 0;
}

void KdOnline2FeInput::GetFrame(int32 frame,
                                VectorBase<float> *feat)
{
    int32 context = delta_opts.order * delta_opts.window;
    int32 left_frame = frame - context;
    int32 right_frame = frame + context;
    int32 src_frames_ready = o_features->Size();
    if (left_frame < 0)
    {
        left_frame = 0;
    }
    if (right_frame >= src_frames_ready)
    {
      right_frame = src_frames_ready - 1;
    }
    KALDI_ASSERT(right_frame >= left_frame);
    int32 temp_num_frames = right_frame + 1 - left_frame;
    int32 mfcc_dim = mfcc->Dim();
    Matrix<BaseFloat> temp_src(temp_num_frames, mfcc_dim);
    for( int32 t=left_frame ; t<=right_frame ; t++ )
    {
        SubVector<BaseFloat> temp_row(temp_src, t - left_frame);
        temp_row.CopyFromVec(*(o_features->At(t)));
    }
    int32 temp_t = frame - left_frame;  // temp_t is the offset of frame "frame"
                                        // within temp_src
    delta_features->Process(temp_src, temp_t, feat);
}

KdOnline2FeInput::~KdOnline2FeInput()
{
    // Guard against double deleting the cmvn_ ptr
    delete cmvn;
    delete mfcc;
}

void KdOnline2FeInput::AcceptWaveform(float sampling_rate,
                                      VectorBase<float> &waveform)
{
    if( waveform.Dim()==0 )
    {
        return;
    }

    Vector<BaseFloat> appended_wave;

    appended_wave.Resize(waveform_remainder_.Dim() + waveform.Dim());
    if (waveform_remainder_.Dim() != 0)
    {
      appended_wave.Range(0, waveform_remainder_.Dim())
          .CopyFromVec(waveform_remainder_);
    }
    appended_wave.Range(waveform_remainder_.Dim(), waveform.Dim())
        .CopyFromVec(waveform);
    waveform_remainder_.Swap(&appended_wave);
    ComputeFeatures();
}

void KdOnline2FeInput::ComputeFeatures()
{
    FeatureWindowFunction window_function(mfcc->GetFrameOptions());
    bool input_finished = false; //always input is not finished
    const FrameExtractionOptions &frame_opts = mfcc->GetFrameOptions();
    int64 num_samples_total = waveform_offset + waveform_remainder_.Dim();
    int32 num_frames_old = o_features->Size();
    int32 num_frames_new = NumFrames(num_samples_total, frame_opts,
                                       input_finished);
    KALDI_ASSERT(num_frames_new >= num_frames_old);

    Vector<BaseFloat> window;
    bool need_raw_log_energy = mfcc->NeedRawLogEnergy();
    for (int32 frame = num_frames_old; frame < num_frames_new; frame++)
    {
        BaseFloat raw_log_energy = 0.0;
        ExtractWindow(waveform_offset, waveform_remainder_, frame,
                      frame_opts, window_function, &window,
                      need_raw_log_energy ? &raw_log_energy : NULL);
        Vector<BaseFloat> *this_feature = new Vector<BaseFloat>(mfcc->Dim(),
                                                                kUndefined);
        // note: this online feature-extraction code does not support VTLN.
        BaseFloat vtln_warp = 1.0;
        mfcc->Compute(raw_log_energy, vtln_warp, &window, this_feature);
        o_features->PushBack(this_feature);
    }
    // OK, we will now discard any portion of the signal that will not be
    // necessary to compute frames in the future.
    int64 first_sample_of_next_frame = FirstSampleOfFrame(num_frames_new,
                                                          frame_opts);
    int32 samples_to_discard = first_sample_of_next_frame - waveform_offset;
    if (samples_to_discard > 0)
    {
        // discard the leftmost part of the waveform that we no longer need.
        int32 new_num_samples = waveform_remainder_.Dim() - samples_to_discard;
        if (new_num_samples <= 0)
        {
            // odd, but we'll try to handle it.
            waveform_offset += waveform_remainder_.Dim();
            waveform_remainder_.Resize(0);
        }
        else
        {
            Vector<BaseFloat> new_remainder(new_num_samples);
            new_remainder.CopyFromVec(waveform_remainder_.Range(samples_to_discard,
                                                                new_num_samples));
            waveform_offset += samples_to_discard;
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
