#ifndef KD_ONLINE_FEINPUT_H
#define KD_ONLINE_FEINPUT_H

#include "bt_config.h"

//#ifndef BT_LAT_ONLINE

#include "online/online-feat-input.h"
#include "feat/feature-mfcc.h"
#include "bt_recorder.h"


class KdOnlineFeInput : public kaldi::OnlineFeatInputItf
{
public:
    // "frame_size" - frame extraction window size in audio samples
    // "frame_shift" - feature frame width in audio samples
    KdOnlineFeInput(BtRecorder *au_src, kaldi::Mfcc *fe,
                    const int32 frame_size, const int32 frame_shift,
                    const bool snip_edges = true);

    int32 Dim() const { return extractor_->Dim(); }

    bool Compute(kaldi::Matrix<kaldi::BaseFloat> *output);
private:

    BtRecorder *source_; // audio source
    kaldi::Mfcc *extractor_; // the actual feature extractor used
    const int32 frame_size_;
    const int32 frame_shift_;
    kaldi::Vector<kaldi::BaseFloat> wave_; // the samples to be passed for extraction
    kaldi::Vector<kaldi::BaseFloat> wave_remainder_; // the samples remained from the previous
    // feature batch
    kaldi::FrameExtractionOptions frame_opts_;
};


//#endif // BT_LAT_ONLINE

#endif // KD_ONLINE_FEINPUT_H
