#ifndef KD_CMVN_H
#define KD_CMVN_H

#include <QObject>
#include <QThread>

#include "kd_cmvn.h"
#include "kd_cmvn_state.h"
#include "matrix/matrix-lib.h"
#include "util/common-utils.h"
#include "base/kaldi-error.h"

class KdCMVN
{
public:

    void GetFrame(int frame,
                  KdRecyclingVector *o_features,
                  kaldi::VectorBase<float> *feat);

    /// If you do have previous utterances from the same speaker
    /// you are supposed to initialize it by calling SetState
    KdCMVN(KdCmvnState &cmvn_state, int dim);

    void GetState(int cur_frame, KdRecyclingVector *o_features,
                  KdCmvnState *cmvn_state);

    // This function can be used to modify the state of the CMVN computation
    // from outside, but must only be called before you have processed any data
    // (otherwise it will crash).
    void SetState(KdCmvnState cmvn_state);
    void Freeze(int cur_frame,
                KdRecyclingVector *o_features);

    virtual ~KdCMVN();
protected:

    /// Smooth by possibly adding some stats from "global_stats"
    /// and/or "speaker_stats", controlled by the config.  The best way to
    /// understand the smoothing rule we use is just to look at the code.
    void SmoothOnlineCmvnStats(const kaldi::MatrixBase<double> &speaker_stats,
                               const kaldi::MatrixBase<double> &global_stats,
                               kaldi::MatrixBase<double> *stats);

    /// Get the most recent cached frame of CMVN stats.  [If no frames
    /// were cached, sets up empty stats for frame zero and returns that].
    void GetMostRecentCachedFrame(int frame,
                                  int *cached_frame,
                                  kaldi::MatrixBase<double> *stats);

    /// Cache this frame of stats.
    void CacheFrame(int frame, const kaldi::MatrixBase<double> &stats);

    /// Initialize ring buffer for caching stats.
    inline void InitRingBufferIfNeeded();

    /// Computes the raw CMVN stats for this frame, making use of (and updating if
    /// necessary) the cached statistics in raw_stats_.  This means the (x,
    /// x^2, count) stats for the last up to opts_.cmn_window frames.
    void ComputeStatsForFrame(int frame, KdRecyclingVector *o_features,
                              kaldi::MatrixBase<double> *stats);

    KdCmvnState orig_state_;   // reflects the state before we saw this
    // utterance.
    kaldi::Matrix<double> frozen_state_;  // If the user called Freeze(), this variable
    // will reflect the CMVN state that we froze
    // at.

    // The variable below reflects the raw (count, x, x^2) statistics of the
    // input, computed every opts_.modulus frames.  raw_stats_[n / opts_.modulus]
    // contains the (count, x, x^2) statistics for the frames from
    // std::max(0, n - opts_.cmn_window) through n.
    std::vector<kaldi::Matrix<double>*> cached_stats_modulo_;
    // the variable below is a ring-buffer of cached stats.  the int is the
    // frame index.
    std::vector<std::pair<int, kaldi::Matrix<double> > > cached_stats_ring_;

    // Some temporary variables used inside functions of this class, which
    // put here to avoid reallocation.
    kaldi::Matrix<double> temp_stats_;
    kaldi::Vector<float>  temp_feats_;
    kaldi::Vector<double> temp_feats_dbl_;

    int cmn_window = 600;
    int speaker_frames = 600;  // must be <= cmn_window
    int global_frames = 200;  // must be <= speaker_frames.
    bool normalize_mean = true;  // Must be true if normalize_variance==true.
    bool normalize_variance = false;
    int Dim; ///REMOVE THIS

    int modulus = 20;  // smaller->more time-efficient but less memory-efficient.
                         // Must be >= 1.
    int ring_buffer_size = 20;  // used for caching CMVN stats.  Must be >=modulus.
};


#endif // BT_CMVN_H
