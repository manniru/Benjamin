#ifndef KD_ONLINE_FEINPUT_H
#define KD_ONLINE_FEINPUT_H

#include "bt_config.h"

//#ifndef BT_LAT_ONLINE

#include "online/online-feat-input.h"
#include "bt_online_source.h"



// Implementation, that is meant to be used to read samples from an
// OnlineAudioSourceItf and to extract MFCC/PLP features in the usual way
template <class E>
class KdOnlineFeInput : public kaldi::OnlineFeatInputItf {
 public:
  // "au_src" - OnlineAudioSourceItf object
  // "fe" - object implementing MFCC/PLP feature extraction
  // "frame_size" - frame extraction window size in audio samples
  // "frame_shift" - feature frame width in audio samples
  KdOnlineFeInput(BtOnlineSource *au_src, E *fe,
                const int32 frame_size, const int32 frame_shift,
                const bool snip_edges = true);

  int32 Dim() const { return extractor_->Dim(); }

  virtual bool Compute(kaldi::Matrix<kaldi::BaseFloat> *output)
  {
      return false;
  }

 private:
  bool CC_F(kaldi::Matrix<kaldi::BaseFloat> *output);

  BtOnlineSource *source_; // audio source
  E *extractor_; // the actual feature extractor used
  const int32 frame_size_;
  const int32 frame_shift_;
  kaldi::Vector<kaldi::BaseFloat> wave_; // the samples to be passed for extraction
  kaldi::Vector<kaldi::BaseFloat> wave_remainder_; // the samples remained from the previous
                                     // feature batch
  kaldi::FrameExtractionOptions frame_opts_;
};


//#endif // BT_LAT_ONLINE

#endif // KD_ONLINE_FEINPUT_H
