#ifndef KD_CMVN_H
#define KD_CMVN_H

#include <QObject>
#include "kd_cmvn_state.h"
#include "bt_cfb.h"

class KdCMVN
{
public:
    KdCMVN(kaldi::Matrix<float> g_state, BtCFB *feat);

    kaldi::Matrix<float> global_state;   // reflects the state before we saw this
    void calc(int frame);

    virtual ~KdCMVN();
protected:

    void addGlobal(kaldi::MatrixBase<double> *stats);

    /// Get the most recent cached frame of CMVN stats.  [If no frames
    /// were cached, sets up empty stats for frame zero and returns that].
    int getLastCached(int frame, kaldi::MatrixBase<double> *stats);

    void CacheFrame(int frame, const kaldi::MatrixBase<double> &stats);
    void InitRingBuffer();

    /// Computes the raw CMVN stats for this frame, making use of (and updating if
    /// necessary) the cached statistics in raw_stats_.  This means the (x,
    /// x^2, count) stats for the last up to opts_.cmn_window frames.
    void updateStats(int frame, KdRecyclingVector *o_features,
                              kaldi::MatrixBase<double> *stats);

    // raw (count, x, x^2) statistics of the input, computed every opts_.modulus frames.  raw_stats_[n / opts_.modulus]
    // contains the (count, x, x^2) statistics for the frames from
    // std::max(0, n - opts_.cmn_window) through n.
    std::vector<kaldi::Matrix<double>*> cached_stats_modulo_;
    // the variable below is a ring-buffer of cached stats.  the int is the
    // frame index.
    std::vector<std::pair<int, kaldi::Matrix<double> > > cached_stats;

    // Some temporary variables used inside functions of this class, which
    // put here to avoid reallocation.
    kaldi::Matrix<double> temp_stats_;
    BtCFB *feature;

    int cmn_window = 600;
    int Dim; ///REMOVE THIS

    int modulus = 20;  // smaller->more time-efficient but less memory-efficient.
                         // Must be >= 1.
    int ring_size = 20;  // Must be >=modulus.
};


#endif // BT_CMVN_H
