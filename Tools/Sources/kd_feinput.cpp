#include "kd_feinput.h"

using namespace kaldi;

KdFeInput::KdFeInput(BtRecorder *au_src, QObject *parent)
    : QObject(parent)
{
    rec_src = au_src;
    waveform_offset = 0;
    mfcc = new KdMFCC;
    delta = new KdDelta;
    o_features = new KdRecyclingVector;

    Init();
}

void KdFeInput::Init()
{
    kaldi::Matrix<float> global_cmvn;
    std::string gcmvn_path = KAL_NATO_DIR"exp/tri1_online/global_cmvn.stats";

    bool binary_in;
    Input ki(gcmvn_path, &binary_in);
    global_cmvn.Read(ki.Stream(), binary_in);
    cmvn = new KdCMVN(global_cmvn, mfcc->Dim());
}

void KdFeInput::resetCmvn()
{
    //    cmvn->resetStat();
}

int KdFeInput::Dim()
{
    int mfcc_dim = mfcc->Dim();
    return mfcc_dim * (1 + KD_DELTA_ORDER);
}

int KdFeInput::NumFramesReady()
{
    int num_frames = o_features->Size();
    // number of frames that is less to produce the output.
    int context = KD_DELTA_ORDER * KD_DELTA_ORDER; //4
    int ret     = num_frames - context;
    if( ret>0 )
    {
        return ret;
    }
    return 0;
}

// Call from outside(decodable)
void KdFeInput::GetFrame(int frame, VectorBase<float> *feat)
{
    int context = KD_DELTA_ORDER * KD_DELTA_WINDOW; //4
    int left_frame = frame - context;
    int right_frame = frame + context;
    int max_frame = o_features->Size();
    if( left_frame<0 )
    {
        left_frame = 0;
    }
    if( right_frame>=max_frame )
    {
        right_frame = max_frame - 1;
    }
    if( right_frame<left_frame )
    {
        qDebug() << "right_frame" << right_frame
                 << "left_frame" << left_frame;
        exit(1);
    }
    int len = right_frame - left_frame + 1 ;
    int mfcc_dim = mfcc->Dim();
    Matrix<float> temp_src(len, mfcc_dim);
    for( int i=0 ; i<len ; i++ )
    {
        SubVector<float> temp_row(temp_src, i);
        temp_row.CopyFromVec(*(o_features->At(left_frame+i)));
        cmvn->GetFrame(left_frame+i, o_features, &temp_row); // normalize mean of temp_row
    }
    int temp_t = frame - left_frame;  // temp_t is the offset of frame "frame"
    // within temp_src
    delta->Process(temp_src, temp_t, feat);
}

KdFeInput::~KdFeInput()
{
    // Guard against double deleting the cmvn_ ptr
    delete cmvn;
    delete mfcc;
}

///sampling_rate is fixed to 16KHz
void KdFeInput::AcceptWaveform(BtCyclic *buf, int len)
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

void KdFeInput::ComputeFeatures()
{
    KdWindow &frame_opts = mfcc->frame_opts;
    int num_samples_total = waveform_offset + waveform_remainder_.Dim();
    int num_frames_old = o_features->Size();
    int num_frames_new = frame_opts.frameCount(num_samples_total);
    KALDI_ASSERT(num_frames_new >= num_frames_old);

    Vector<float> window;
    for( int frame=num_frames_old ; frame<num_frames_new ; frame++)
    {
        frame_opts.ExtractWindow(waveform_offset, waveform_remainder_, frame,
                                 &window, NULL); //dont need energy

        Vector<float> *features = new Vector<float>(mfcc->Dim(), kUndefined);

        mfcc->Compute(&window, features);
        o_features->PushBack(features);
    }
    // OK, we will now discard any portion of the signal that will not be
    // necessary to compute frames in the future.
    int first_sample_of_next_frame = frame_opts.FirstSampleOfFrame(num_frames_new);
    int samples_to_discard = first_sample_of_next_frame - waveform_offset;
    if( samples_to_discard > 0)
    {
        // discard the leftmost part of the waveform that we no longer need.
        int new_num_samples = waveform_remainder_.Dim() - samples_to_discard;
        if( new_num_samples <= 0)
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
