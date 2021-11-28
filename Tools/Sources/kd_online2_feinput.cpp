#include "kd_online2_feinput.h"

using namespace kaldi;

KdOnline2FeInput::KdOnline2FeInput(BtRecorder *au_src, QObject *parent)
                : QObject(parent)
{
    rec_src = au_src;
    waveform_offset = 0;
    std::string  global_cmvn_stats_rxfilename = KAL_NATO_DIR"exp/tri1_online/global_cmvn.stats";
    mfcc_opts.use_energy = false;

    ReadKaldiObject(global_cmvn_stats_rxfilename,
                    &global_cmvn_stats_);
    Init();

    if( rec_src )
    {
        rec_src->startStream();
//        rec_thread = new QThread;
//        rec_src->moveToThread(rec_thread);
//        rec_thread->start();
    }
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
    cmvn = new KdCMVN(cmvn_opts, initial_state, mfcc->Dim());

    delta_features = new DeltaFeatures(delta_opts);
}

void KdOnline2FeInput::FreezeCmvn()
{
    cmvn->Freeze(o_features->Size() - 1, o_features);
}

int32 KdOnline2FeInput::Dim()
{
    int32 mfcc_dim = mfcc->Dim();
    return mfcc_dim * (1 + delta_opts.order);
}

int32 KdOnline2FeInput::NumFramesReady()
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
    if( rec_src )
    {
        AcceptWaveform(rec_src->cy_buf);
    }

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
    if( right_frame<left_frame );
    {
//        qDebug() << "right_frame" << right_frame
//                 << "left_frame" << left_frame;
        return;
    }
    int32 temp_num_frames = right_frame + 1 - left_frame;
    int32 mfcc_dim = mfcc->Dim();
    Matrix<BaseFloat> temp_src(temp_num_frames, mfcc_dim);
    for( int32 t=left_frame ; t<=right_frame ; t++ )
    {
        SubVector<BaseFloat> temp_row(temp_src, t - left_frame);
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
void KdOnline2FeInput::AcceptWaveform(int16_t *data, int size)
{
    if( size==0 )
    {
        return;
    }

    Vector<float> appended_wave;
    int rem_size = waveform_remainder_.Dim();

    appended_wave.Resize(rem_size + size);
    if( rem_size!=0 )
    {
        appended_wave.Range(0, waveform_remainder_.Dim())
                .CopyFromVec(waveform_remainder_);
    }
    for( int i=0 ; i<size ; i++ )
    {
        appended_wave(i+rem_size) = data[i];
    }
    waveform_remainder_.Swap(&appended_wave);
    ComputeFeatures();
}

///sampling_rate is fixed to 16KHz
void KdOnline2FeInput::AcceptWaveform(BtCyclic *buf)
{
    int len = buf->getDataSize();
    if( len==0 )
    {
        return;
    }
    qDebug() << "hey";

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
    FeatureWindowFunction window_function(mfcc->GetFrameOptions());
    bool input_finished = false; //always input is not finished
    const FrameExtractionOptions &frame_opts = mfcc->GetFrameOptions();
    int64 num_samples_total = waveform_offset + waveform_remainder_.Dim();
    int32 num_frames_old = o_features->Size();
    int32 num_frames_new = NumFrames(num_samples_total, frame_opts,
                                       input_finished);
    KALDI_ASSERT(num_frames_new >= num_frames_old);

    Vector<BaseFloat> window;
    for (int32 frame = num_frames_old; frame < num_frames_new; frame++)
    {
        ExtractWindow(waveform_offset, waveform_remainder_, frame,
                      frame_opts, window_function, &window, NULL); //dont need energy

        Vector<BaseFloat> *this_feature = new Vector<BaseFloat>(mfcc->Dim(),
                                                                kUndefined);

        float vtln_warp = 1.0; // this code does not support VTLN.
        float raw_log_energy = 0.0;
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
