#ifndef KD_ONLINE2_FEINPUT_H
#define KD_ONLINE2_FEINPUT_H

#include "bt_config.h"

//#ifndef BT_LAT_ONLINE

#include "online2/online-feature-pipeline.h"
#include "feat/feature-mfcc.h"
#include "bt_recorder.h"
#include "kd_cmvn.h"

class KdOnline2FeInput
{
public:
  KdOnline2FeInput();

  int32 Dim();
  int32 NumFramesReady();
  void GetFrame(int32 frame, kaldi::VectorBase<float> *feat);

  void FreezeCmvn();  // stop it from moving further (do this when you start
                      // using fMLLR). This will crash if NumFramesReady() == 0.

  /// Accept more data to process (won't actually process it, will
  /// just copy it).   sampling_rate is necessary just to assert
  /// it equals what's in the config.
  void AcceptWaveform(kaldi::VectorBase<float> &waveform);

  // InputFinished() tells the class you won't be providing any
  // more waveform.  This will help flush out the last few frames
  // of delta or LDA features, and finalize the pitch features
  // (making them more accurate).
  void InputFinished();

  void ComputeFeatures();

  virtual ~KdOnline2FeInput();

 private:
  /// Init() is to be called from the constructor; it assumes the pointer
  /// members are all uninitialized but config_ and lda_mat_ are
  /// initialized.
  void Init();

  kaldi::Matrix<float> lda_mat_;  // LDA matrix, if supplied.
  kaldi::Matrix<float> global_cmvn_stats_;  // Global CMVN stats.

  KdCMVN *cmvn;
  kaldi::MfccOptions mfcc_opts;  // options for MFCC computation,
  kaldi::OnlineCmvnOptions cmvn_opts;  // Options for online CMN/CMVN computation.
  kaldi::DeltaFeaturesOptions delta_opts;  // Options for delta computation, if done.

  /////////////////////////////
  kaldi::RecyclingVector *o_features;
  kaldi::MfccComputer *mfcc;

  // waveform_remainder_ is a short piece of waveform that we may need to keep
  // after extracting all the whole frames we can (whatever length of feature
  // will be required for the next phase of computation).
  kaldi::Vector<float> waveform_remainder_;

  // waveform_offset_ is the number of samples of waveform that we have
  // already discarded, i.e. that were prior to 'waveform_remainder_'.
  int64 waveform_offset;


  kaldi::DeltaFeatures *delta_features;  // This class contains just a few
                                        // coefficients.
};

#endif // KD_ONLINE2_FEINPUT_H
