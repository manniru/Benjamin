#include "kd_online_feinput.h"

//#ifndef BT_LAT_ONLINE

#include "QDebug"

using namespace kaldi;


KdOnlineFeInput::KdOnlineFeInput(BtRecorder *au_src, Mfcc *fe, int32 frame_size,
                                 int32 frame_shift, bool snip_edges)
    : source_(au_src), extractor_(fe),
      frame_size_(frame_size), frame_shift_(frame_shift)
{
      // we need a FrameExtractionOptions to call NumFrames()
      // 1000 is just a fake sample rate which equates ms and samples
      frame_opts_.samp_freq = 1000;
      frame_opts_.frame_shift_ms = frame_shift;
      frame_opts_.frame_length_ms = frame_size;
      frame_opts_.snip_edges = snip_edges;
}

bool KdOnlineFeInput::Compute(Matrix<float> *output)
{
    MatrixIndexT nvec = output->NumRows(); // the number of output vectors
    if( nvec<=0 )
    {
        qDebug() << "No feature vectors requested?!";
        return true;
    }

    // Prepare the input audio samples
    int32 samples_req = frame_size_ + (nvec - 1) * frame_shift_;
    Vector<float> read_samples(samples_req);

    bool ans = source_->Read(&read_samples);

    Vector<float> all_samples(wave_remainder_.Dim() + read_samples.Dim());
    all_samples.Range(0, wave_remainder_.Dim()).CopyFromVec(wave_remainder_);
    all_samples.Range(wave_remainder_.Dim(), read_samples.Dim()).
                      CopyFromVec(read_samples);

    // Extract the features
    if( all_samples.Dim()>=frame_size_ )
    {
        // extract waveform remainder before calling Compute()
        int32 num_frames = NumFrames(all_samples.Dim(), frame_opts_);
        // offset is the amount at the start that has been extracted.
        int32 offset = num_frames * frame_shift_;
        int32 remaining_len = all_samples.Dim() - offset;
        wave_remainder_.Resize( remaining_len );
        KALDI_ASSERT( remaining_len>=0 );
        if( remaining_len>0 )
        {
            wave_remainder_.CopyFromVec(SubVector<float>
                                       (all_samples, offset, remaining_len));
        }
        extractor_->Compute(all_samples, 1.0, output);
    }
    else
    {
        output->Resize(0, 0);
        wave_remainder_ = all_samples;
    }

    return ans;
}
//#endif // BT_LAT_ONLINE
