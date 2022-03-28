#ifndef KD_CMVN_H
#define KD_CMVN_H

#include <QObject>
#include "kd_cmvn_state.h"

class KdCMVN
{
public:
    /// If you do have previous utterances from the same speaker
    /// you are supposed to initialize it by calling SetState
    KdCMVN(kaldi::Matrix<double> cmvn_state, int dim);

    kaldi::Matrix<double> global_state;   // reflects the state before we saw this
    void GetFrame(int frame,
                  KdRecyclingVector *o_features,
                  kaldi::VectorBase<float> *feat);

    virtual ~KdCMVN();
protected:

    /// Smooth by possibly adding some stats from "global_stats"
    /// and/or "speaker_stats", controlled by the config.  The best way to
    /// understand the smoothing rule we use is just to look at the code.
    void smoothStats(kaldi::MatrixBase<double> *stats);

    /// Get the most recent cached frame of CMVN stats.  [If no frames
    /// were cached, sets up empty stats for frame zero and returns that].
    int getLastCachedFrame(int frame,
                                  kaldi::MatrixBase<double> *stats);

    /// Cache this frame of stats.
    void CacheFrame(int frame, const kaldi::MatrixBase<double> &stats);

    /// Initialize ring buffer for caching stats.
    inline void InitRingBuffer();

    /// Computes the raw CMVN stats for this frame, making use of (and updating if
    /// necessary) the cached statistics in raw_stats_.  This means the (x,
    /// x^2, count) stats for the last up to opts_.cmn_window frames.
    void ComputeStats(int frame, KdRecyclingVector *o_features,
                              kaldi::MatrixBase<double> *stats);

    // utterance.

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

    int cmn_window = 2000;
    int global_frames = 200;  // not used much
    int Dim; ///REMOVE THIS

    int modulus = 20;  // smaller->more time-efficient but less memory-efficient.
                         // Must be >= 1.
    int ring_buffer_size = 20;  // used for caching CMVN stats.  Must be >=modulus.
};


#endif // BT_CMVN_H
