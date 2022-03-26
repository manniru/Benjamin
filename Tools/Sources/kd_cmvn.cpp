#include "kd_cmvn.h"
#include <QDebug>

#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <matrix/kaldi-matrix.h>

using namespace kaldi;

KdCMVN::KdCMVN(Matrix<double> cmvn_state,
               int dim)
{
    temp_feats_dbl_.Resize(dim);
    temp_feats_.Resize(dim);
    temp_stats_.Resize(2, dim + 1);
    state_ = cmvn_state;
    Dim = dim;
}

void KdCMVN::GetMostRecentCachedFrame(int frame,
                                          int *cached_frame,
                                          MatrixBase<double> *stats)
{
    KALDI_ASSERT(frame >= 0);
    InitRingBuffer();
    // look for a cached frame on a previous frame as close as possible in time
    // to "frame".  Return if we get one.
    for (int t = frame; t >= 0 && t >= frame - ring_buffer_size; t--)
    {
        if( t % modulus == 0)
        {
            // if this frame should be cached in cached_stats_modulo_, then
            // we'll look there, and we won't go back any further in time.
            break;
        }
        int index = t % ring_buffer_size;
        if( cached_stats_ring_[index].first == t)
        {
            *cached_frame = t;
            stats->CopyFromMat(cached_stats_ring_[index].second);
            return;
        }
    }
    int n = frame / modulus;
    if( n >= cached_stats_modulo_.size())
    {
        if( cached_stats_modulo_.size() == 0)
        {
            *cached_frame = -1;
            stats->SetZero();
            return;
        }
        else
        {
            n = static_cast<int>(cached_stats_modulo_.size() - 1);
        }
    }
    *cached_frame = n * modulus;
    KALDI_ASSERT(cached_stats_modulo_[n] != NULL);
    stats->CopyFromMat(*(cached_stats_modulo_[n]));
}

// Initialize ring buffer for caching stats.
void KdCMVN::InitRingBuffer()
{
    if( cached_stats_ring_.empty() && ring_buffer_size>0 )
    {
        Matrix<double> temp(2, Dim + 1);
        cached_stats_ring_.resize(ring_buffer_size,
                                  std::pair<int, Matrix<double> >(-1, temp));
    }
}

void KdCMVN::CacheFrame(int frame, const MatrixBase<double> &stats)
{
    KALDI_ASSERT(frame >= 0);
    if( frame % modulus == 0)
    {
        int n = frame / modulus;
        if( n >= cached_stats_modulo_.size())
        {
            // The following assert is a limitation on in what order you can call
            // CacheFrame.  Fortunately the calling code always calls it in sequence,
            // which it has to because you need a previous frame to compute the
            // current one.
            KALDI_ASSERT(n == cached_stats_modulo_.size());
            cached_stats_modulo_.push_back(new Matrix<double>(stats));
        }
        else
        {
            KALDI_WARN << "Did not expect to reach this part of code.";
            // do what seems right, but we shouldn't get here.
            cached_stats_modulo_[n]->CopyFromMat(stats);
        }
    }
    else
    {  // store in the ring buffer.
        InitRingBuffer();
        if( !cached_stats_ring_.empty())
        {
            int index = frame % cached_stats_ring_.size();
            cached_stats_ring_[index].first = frame;
            cached_stats_ring_[index].second.CopyFromMat(stats);
        }
    }
}

KdCMVN::~KdCMVN()
{
    for (size_t i = 0; i < cached_stats_modulo_.size(); i++)
    {
        delete cached_stats_modulo_[i];
    }
    cached_stats_modulo_.clear();
}

void KdCMVN::ComputeStatsForFrame(int frame,
                                  KdRecyclingVector *o_features,
                                  MatrixBase<double> *stats_out)
{
    KALDI_ASSERT(frame >= 0 && frame < o_features->Size());

    int cur_frame;
    GetMostRecentCachedFrame(frame, &cur_frame, stats_out);

    Vector<float> &feats(temp_feats_);
    Vector<double> &feats_dbl(temp_feats_dbl_);
    while (cur_frame < frame)
    {
        cur_frame++;
        feats.CopyFromVec(*(o_features->At(cur_frame)));////GET FRAME
        feats_dbl.CopyFromVec(feats);
        stats_out->Row(0).Range(0, Dim).AddVec(1.0, feats_dbl);
        (*stats_out)(0, Dim) += 1.0;
        // it's a sliding buffer; a frame at the back may be
        // leaving the buffer so we have to subtract that.
        int prev_frame = cur_frame - cmn_window;
        if( prev_frame >= 0)
        {
            // we need to subtract frame prev_f from the stats.
            feats.CopyFromVec(*(o_features->At(prev_frame)));////GET FRAME
            feats_dbl.CopyFromVec(feats);
            stats_out->Row(0).Range(0, Dim).AddVec(-1.0, feats_dbl);
            (*stats_out)(0, Dim) -= 1.0;
        }
        CacheFrame(cur_frame, (*stats_out));
    }
}


// static
void KdCMVN::SmoothOnlineCmvnStats(const MatrixBase<double> &global_stats,
                                   MatrixBase<double> *stats)
{
    int dim = stats->NumCols() - 1;
    double cur_count = (*stats)(0, dim);
    // If count exceeded cmn_window it would be an error in how "window_stats"
    // was accumulated.
    KALDI_ASSERT(cur_count <= 1.001 * cmn_window);
    if( cur_count >= cmn_window)
        return;
    if( cur_count >= cmn_window)
        return;
    if( global_stats.NumRows()==0 )
    {
        KALDI_ERR << "Global CMN stats are required";
    }

    double count_from_global = cmn_window - cur_count,
            global_count = global_stats(0, dim);
    KALDI_ASSERT(global_count > 0.0);
    if( count_from_global > global_frames)
        count_from_global = global_frames;
    if( count_from_global > 0.0)
        stats->AddMat(count_from_global / global_count,
                      global_stats);
}

void KdCMVN::GetFrame(int frame,
                      KdRecyclingVector *o_features,
                      VectorBase<float> *feat)
{
    KALDI_ASSERT(feat->Dim() == Dim);
    Matrix<double> &stats(temp_stats_);
    stats.Resize(2, Dim + 1, kUndefined);  // Will do nothing if size was correct.
    // first get the raw CMVN stats (this involves caching..)
    ComputeStatsForFrame(frame, o_features, &stats);
    // now smooth them.
    SmoothOnlineCmvnStats(state_,
                          &stats);

    SubMatrix<float> feat_mat(feat->Data(), 1, Dim, Dim);
    // the function ApplyCmvn takes a matrix, so form a one-row matrix
    // to give it.
    if( normalize_mean )
    {
        kd_applyCmvn(stats, &feat_mat);
    }
}
