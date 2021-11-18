#ifndef KD_ONLINE_FEINPUT_H
#define KD_ONLINE_FEINPUT_H

#include "bt_config.h"

//#ifndef BT_LAT_ONLINE

#include "online/online-feat-input.h"
#include "feat/feature-mfcc.h"
#include "bt_recorder.h"
class OnlineFeaturePipeline: public OnlineFeatureInterface
{
public:
  explicit OnlineFeaturePipeline(const OnlineFeaturePipelineConfig &cfg);

  /// Member functions from OnlineFeatureInterface:
  virtual int32 Dim() const;
  virtual bool IsLastFrame(int32 frame) const;
  virtual int32 NumFramesReady() const;
  virtual void GetFrame(int32 frame, VectorBase<BaseFloat> *feat);

  // This is supplied for debug purposes.
  void GetAsMatrix(Matrix<BaseFloat> *feats);

  void FreezeCmvn();  // stop it from moving further (do this when you start
                      // using fMLLR). This will crash if NumFramesReady() == 0.

  /// Set the CMVN state to a particular value (will generally be
  /// called after Copy().
  void SetCmvnState(const OnlineCmvnState &cmvn_state);
  void GetCmvnState(OnlineCmvnState *cmvn_state);

  /// Accept more data to process (won't actually process it, will
  /// just copy it).   sampling_rate is necessary just to assert
  /// it equals what's in the config.
  void AcceptWaveform(BaseFloat sampling_rate,
                      const VectorBase<BaseFloat> &waveform);

  BaseFloat FrameShiftInSeconds() const {
    return config_.FrameShiftInSeconds();
  }

  // InputFinished() tells the class you won't be providing any
  // more waveform.  This will help flush out the last few frames
  // of delta or LDA features, and finalize the pitch features
  // (making them more accurate).
  void InputFinished();

  // This object is used to set the fMLLR transform.  Call it with
  // the empty matrix if you want to stop it using any transform.
  void SetTransform(const MatrixBase<BaseFloat> &transform);


  // Returns true if an fMLLR transform has been set using
  // SetTransform().
  bool HaveFmllrTransform() { return fmllr_ != NULL; }

  /// returns a newly initialized copy of *this-- this does not duplicate all
  /// the internal state or the speaker-adaptation state, but gives you a
  /// freshly initialized version of this object, as if you had initialized it
  /// using the constructor that takes the config file.  After calling this you
  /// may want to call SetCmvnState() and SetTransform().
  OnlineFeaturePipeline *New() const;

  virtual ~OnlineFeaturePipeline();

 private:
  /// The following constructor is used internally in the New() function;
  /// it has the same effect as initializing from just "cfg", but avoids
  /// re-reading the LDA transform from disk.
  OnlineFeaturePipeline(const OnlineFeaturePipelineConfig &cfg,
                        const Matrix<BaseFloat> &lda_mat,
                        const Matrix<BaseFloat> &global_cmvn_stats);

  /// Init() is to be called from the constructor; it assumes the pointer
  /// members are all uninitialized but config_ and lda_mat_ are
  /// initialized.
  void Init();

  OnlineFeaturePipelineConfig config_;
  Matrix<BaseFloat> lda_mat_;  // LDA matrix, if supplied.
  Matrix<BaseFloat> global_cmvn_stats_;  // Global CMVN stats.

  OnlineBaseFeature *base_feature_;        // MFCC/PLP/Fbank
  OnlinePitchFeature *pitch_;              // Raw pitch
  OnlineProcessPitch *pitch_feature_;  // Processed pitch
  OnlineFeatureInterface *feature_;        // CMVN (+ processed pitch)

  OnlineCmvn *cmvn_;
  OnlineFeatureInterface *splice_or_delta_;  // This may be NULL if we're not
                                             // doing splicing or deltas.

  OnlineFeatureInterface *lda_;  // If non-NULL, the LDA or LDA+MLLT transform.

  /// returns lda_ if it exists, else splice_or_delta_, else cmvn_.  If this
  /// were not private we would have const and non-const versions returning
  /// const and non-const pointers.
  OnlineFeatureInterface* UnadaptedFeature() const;

  OnlineFeatureInterface *fmllr_;  // non-NULL if we currently have an fMLLR
                                   // transform.

  /// returns adapted feature if fmllr_ exists, else UnadaptedFeature().  If
  /// this were not private we would have const and non-const versions returning
  /// const and non-const pointers.
  OnlineFeatureInterface* AdaptedFeature() const;
};

#endif // KD_ONLINE_FEINPUT_H
