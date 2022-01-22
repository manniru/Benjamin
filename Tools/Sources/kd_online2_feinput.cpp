#include "kd_online2_feinput.h"

using namespace kaldi;

KdOnline2FeInput::KdOnline2FeInput(BtRecorder *au_src, QObject *parent)
                : QObject(parent)
{
    rec_src = au_src;
    waveform_offset = 0;
    std::string gcmvn__path = KAL_NATO_DIR"exp/tri1_online/global_cmvn.stats";

    kd_readMatrix(gcmvn__path, &global_cmvn_stats_);
    Init();
}

// Init() is to be called from the constructor; it assumes the pointer
// members are all uninitialized
void KdOnline2FeInput::Init()
{
    mfcc = new KdMFCC;
    o_features = new RecyclingVector;
    KALDI_ASSERT(global_cmvn_stats_.NumRows() != 0);
    Matrix<double> global_cmvn_stats_dbl(global_cmvn_stats_);
    OnlineCmvnState initial_state(global_cmvn_stats_dbl);
    cmvn = new KdCMVN(cmvn_opts, initial_state, mfcc->Dim());

    delta_features = new DeltaFeatures(delta_opts);
}

void KdOnline2FeInput::FreezeCmvn()
{
    cmvn->Freeze(o_features->Size() - 1, o_features);
}

int KdOnline2FeInput::Dim()
{
    int mfcc_dim = mfcc->Dim();
    return mfcc_dim * (1 + delta_opts.order);
}

int KdOnline2FeInput::NumFramesReady()
{
    int num_frames = o_features->Size();
    // number of frames that is less to produce the output.
    int context = delta_opts.order * delta_opts.window;
    int ret     = num_frames - context;
    if( ret>0 )
    {
        return ret;
    }
    return 0;
}

void KdOnline2FeInput::GetFrame(int frame,
                                VectorBase<float> *feat)
{
    int context = delta_opts.order * delta_opts.window;
    int left_frame = frame - context;
    int right_frame = frame + context;
    int src_frames_ready = o_features->Size();
    if (left_frame < 0)
    {
        left_frame = 0;
    }
    if (right_frame >= src_frames_ready)
    {
      right_frame = src_frames_ready - 1;
    }
    if( right_frame<left_frame )
    {
        qDebug() << "right_frame" << right_frame
                 << "left_frame" << left_frame;
        return;
    }
    int temp_num_frames = right_frame + 1 - left_frame;
    int mfcc_dim = mfcc->Dim();
    Matrix<float> temp_src(temp_num_frames, mfcc_dim);
    for( int t=left_frame ; t<=right_frame ; t++ )
    {
        SubVector<float> temp_row(temp_src, t - left_frame);
        temp_row.CopyFromVec(*(o_features->At(t)));////GET FRAME
        cmvn->GetFrame(t, o_features, &temp_row);
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

///sampling_rate is fixed to 16KHz
void KdOnline2FeInput::AcceptWaveform(BtCyclic *buf, int len)
{
    if( len==0 )
    {
        len = buf->getDataSize()-10;
        if( len<=0 )
        {
            return;
        }
    }

    Vector<float> appended_wave;
    int rem_size = waveform_remainder_.Dim();

    appended_wave.Resize(rem_size + len);
    if( rem_size!=0 )
    {
        appended_wave.Range(0, waveform_remainder_.Dim())
                .CopyFromVec(waveform_remainder_);
    }
    buf->read(&appended_wave, len);
    waveform_remainder_.Swap(&appended_wave);
    ComputeFeatures();
}

void KdOnline2FeInput::ComputeFeatures()
{
    KdWindow &frame_opts = mfcc->frame_opts;
    int64 num_samples_total = waveform_offset + waveform_remainder_.Dim();
    int32 num_frames_old = o_features->Size();
    int32 num_frames_new = frame_opts.NumFrames(num_samples_total);
    KALDI_ASSERT(num_frames_new >= num_frames_old);

    Vector<float> window;
    for (int32 frame = num_frames_old; frame < num_frames_new; frame++)
    {
        frame_opts.ExtractWindow(waveform_offset, waveform_remainder_, frame,
                      &window, NULL); //dont need energy

        Vector<float> *this_feature = new Vector<float>(mfcc->Dim(),
                                                                kUndefined);

        float raw_log_energy = 0.0;
        mfcc->Compute(raw_log_energy, &window, this_feature);
        o_features->PushBack(this_feature);
    }
    // OK, we will now discard any portion of the signal that will not be
    // necessary to compute frames in the future.
    int64 first_sample_of_next_frame = frame_opts.FirstSampleOfFrame(num_frames_new);
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
            Vector<float> new_remainder(new_num_samples);
            new_remainder.CopyFromVec(waveform_remainder_.Range(samples_to_discard,
                                                                new_num_samples));
            waveform_offset += samples_to_discard;
            waveform_remainder_.Swap(&new_remainder);
        }
    }
}
