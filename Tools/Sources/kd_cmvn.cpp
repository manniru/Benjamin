#include "kd_cmvn.h"
#include <QDebug>

#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <matrix/kaldi-matrix.h>
#include <transform/cmvn.h>

using namespace kaldi;

KdCMVN::KdCMVN(KdCmvnState &cmvn_state,
               int dim)
{
    temp_feats_dbl_.Resize(dim);
    temp_feats_.Resize(dim);
    temp_stats_.Resize(2, dim + 1);
    SetState(cmvn_state);
    Dim = dim;
}


void KdCMVN::GetMostRecentCachedFrame(int frame,
                                          int *cached_frame,
                                          MatrixBase<double> *stats)
{
    KALDI_ASSERT(frame >= 0);
    InitRingBufferIfNeeded();
    // look for a cached frame on a previous frame as close as possible in time
    // to "frame".  Return if we get one.
    for (int t = frame; t >= 0 && t >= frame - ring_buffer_size; t--)
    {
        if (t % modulus == 0)
        {
            // if this frame should be cached in cached_stats_modulo_, then
            // we'll look there, and we won't go back any further in time.
            break;
        }
        int index = t % ring_buffer_size;
        if (cached_stats_ring_[index].first == t)
        {
            *cached_frame = t;
            stats->CopyFromMat(cached_stats_ring_[index].second);
            return;
        }
    }
    int n = frame / modulus;
    if (n >= cached_stats_modulo_.size())
    {
        if (cached_stats_modulo_.size() == 0)
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
void KdCMVN::InitRingBufferIfNeeded()
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
    if (frame % modulus == 0)
    {
        int n = frame / modulus;
        if (n >= cached_stats_modulo_.size())
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
        InitRingBufferIfNeeded();
        if (!cached_stats_ring_.empty())
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
        if (normalize_variance)
            stats_out->Row(1).Range(0, Dim).AddVec2(1.0, feats_dbl);
        (*stats_out)(0, Dim) += 1.0;
        // it's a sliding buffer; a frame at the back may be
        // leaving the buffer so we have to subtract that.
        int prev_frame = cur_frame - cmn_window;
        if (prev_frame >= 0)
        {
            // we need to subtract frame prev_f from the stats.
            feats.CopyFromVec(*(o_features->At(prev_frame)));////GET FRAME
            feats_dbl.CopyFromVec(feats);
            stats_out->Row(0).Range(0, Dim).AddVec(-1.0, feats_dbl);
            if (normalize_variance)
                stats_out->Row(1).Range(0, Dim).AddVec2(-1.0, feats_dbl);
            (*stats_out)(0, Dim) -= 1.0;
        }
        CacheFrame(cur_frame, (*stats_out));
    }
}


// static
void KdCMVN::SmoothOnlineCmvnStats(const MatrixBase<double> &speaker_stats,
                                   const MatrixBase<double> &global_stats,
                                   MatrixBase<double> *stats)
{
    if (speaker_stats.NumRows() == 2 && !normalize_variance)
    {
        // this is just for efficiency: don't operate on the variance if it's not
        // needed.
        int cols = speaker_stats.NumCols();  // dim + 1
        SubMatrix<double> stats_temp(*stats, 0, 1, 0, cols);
        SmoothOnlineCmvnStats(speaker_stats.RowRange(0, 1),
                              global_stats.RowRange(0, 1), &stats_temp);
        return;
    }
    int dim = stats->NumCols() - 1;
    double cur_count = (*stats)(0, dim);
    // If count exceeded cmn_window it would be an error in how "window_stats"
    // was accumulated.
    KALDI_ASSERT(cur_count <= 1.001 * cmn_window);
    if (cur_count >= cmn_window)
        return;
    if (speaker_stats.NumRows() != 0)
    {  // if we have speaker stats..
        double count_from_speaker = cmn_window - cur_count;
        double speaker_count = speaker_stats(0, dim);
        if (count_from_speaker > speaker_frames)
            count_from_speaker = speaker_frames;
        if (count_from_speaker > speaker_count)
            count_from_speaker = speaker_count;
        if (count_from_speaker > 0.0)
            stats->AddMat(count_from_speaker / speaker_count,
                          speaker_stats);
        cur_count = (*stats)(0, dim);
    }
    if (cur_count >= cmn_window)
        return;
    if (global_stats.NumRows() != 0)
    {
        double count_from_global = cmn_window - cur_count,
                global_count = global_stats(0, dim);
        KALDI_ASSERT(global_count > 0.0);
        if (count_from_global > global_frames)
            count_from_global = global_frames;
        if (count_from_global > 0.0)
            stats->AddMat(count_from_global / global_count,
                          global_stats);
    }
    else
    {
        KALDI_ERR << "Global CMN stats are required";
    }
}

void KdCMVN::GetFrame(int frame,
                      KdRecyclingVector *o_features,
                      VectorBase<float> *feat)
{
    KALDI_ASSERT(feat->Dim() == Dim);
    Matrix<double> &stats(temp_stats_);
    stats.Resize(2, Dim + 1, kUndefined);  // Will do nothing if size was correct.
    if( frozen_state_.NumRows()!=0 )
    {  // the CMVN state has been frozen.
        stats.CopyFromMat(frozen_state_);
    }
    else
    {
        // first get the raw CMVN stats (this involves caching..)
        ComputeStatsForFrame(frame, o_features, &stats);
        // now smooth them.
        SmoothOnlineCmvnStats(orig_state_.speaker_cmvn_stats,
                              orig_state_.global_cmvn_stats,
                              &stats);
    }

    // call the function ApplyCmvn declared in ../transform/cmvn.h, which
    // requires a matrix.
    // 1 row; num-cols == dim; stride  == dim.
    SubMatrix<float> feat_mat(feat->Data(), 1, Dim, Dim);
    // the function ApplyCmvn takes a matrix, so form a one-row matrix to give it.
    if( normalize_mean )
    {
        ApplyCmvn(stats, normalize_variance, &feat_mat);
    }
    else
    {
        KALDI_ASSERT(!normalize_variance);
    }
}

void KdCMVN::Freeze(int cur_frame, KdRecyclingVector *o_features)
{
    Matrix<double> stats(2, Dim + 1);
    // get the raw CMVN stats
    ComputeStatsForFrame(cur_frame, o_features, &stats);
    // now smooth them.
    SmoothOnlineCmvnStats(orig_state_.speaker_cmvn_stats,
                          orig_state_.global_cmvn_stats,
                          &stats);
    this->frozen_state_ = stats;
}

void KdCMVN::GetState(int cur_frame,
                      KdRecyclingVector *o_features,
                      KdCmvnState *state_out)
{
    *state_out = this->orig_state_;
    { // This block updates state_out->speaker_cmvn_stats
        if (state_out->speaker_cmvn_stats.NumRows() == 0)
            state_out->speaker_cmvn_stats.Resize(2, Dim + 1);
        Vector<float> feat(Dim);
        Vector<double> feat_dbl(Dim);
        for (int t = 0; t <= cur_frame; t++)
        {
            feat.CopyFromVec(*(o_features->At(t)));////GET FRAME
            feat_dbl.CopyFromVec(feat);
            state_out->speaker_cmvn_stats(0, Dim) += 1.0;
            state_out->speaker_cmvn_stats.Row(0).Range(0, Dim).AddVec(1.0, feat_dbl);
            state_out->speaker_cmvn_stats.Row(1).Range(0, Dim).AddVec2(1.0, feat_dbl);
        }
    }
    // Store any frozen state (the effect of the user possibly
    // having called Freeze().
    state_out->frozen_state = frozen_state_;
}

void KdCMVN::SetState(KdCmvnState cmvn_state)
{
    KALDI_ASSERT(cached_stats_modulo_.empty() &&
                 "You cannot call SetState() after processing data.");
    orig_state_ = cmvn_state;
    frozen_state_ = cmvn_state.frozen_state;
}
