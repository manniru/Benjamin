#include "kd_cmvn.h"
#include <QDebug>

#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <matrix/kaldi-matrix.h>
#include <transform/cmvn.h>

using namespace kaldi;



KdCMVN::KdCMVN(OnlineCmvnOptions &opts,
               OnlineCmvnState &cmvn_state,
               OnlineFeatureInterface *src):
    opts_(opts), temp_stats_(2, src->Dim() + 1),
    temp_feats_(src->Dim()), temp_feats_dbl_(src->Dim()),
    src_(src)
{
    SetState(cmvn_state);
    if (!SplitStringToIntegers(opts.skip_dims, ":", false, &skip_dims_))
        KALDI_ERR << "Bad --skip-dims option (should be colon-separated list of "
                  <<  "integers)";
}


void KdCMVN::GetMostRecentCachedFrame(int32 frame,
                                          int32 *cached_frame,
                                          MatrixBase<double> *stats)
{
    KALDI_ASSERT(frame >= 0);
    InitRingBufferIfNeeded();
    // look for a cached frame on a previous frame as close as possible in time
    // to "frame".  Return if we get one.
    for (int32 t = frame; t >= 0 && t >= frame - opts_.ring_buffer_size; t--)
    {
        if (t % opts_.modulus == 0)
        {
            // if this frame should be cached in cached_stats_modulo_, then
            // we'll look there, and we won't go back any further in time.
            break;
        }
        int32 index = t % opts_.ring_buffer_size;
        if (cached_stats_ring_[index].first == t)
        {
            *cached_frame = t;
            stats->CopyFromMat(cached_stats_ring_[index].second);
            return;
        }
    }
    int32 n = frame / opts_.modulus;
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
            n = static_cast<int32>(cached_stats_modulo_.size() - 1);
        }
    }
    *cached_frame = n * opts_.modulus;
    KALDI_ASSERT(cached_stats_modulo_[n] != NULL);
    stats->CopyFromMat(*(cached_stats_modulo_[n]));
}

// Initialize ring buffer for caching stats.
void KdCMVN::InitRingBufferIfNeeded()
{
    if( cached_stats_ring_.empty() && opts_.ring_buffer_size>0 )
    {
        Matrix<double> temp(2, this->Dim() + 1);
        cached_stats_ring_.resize(opts_.ring_buffer_size,
                                  std::pair<int32, Matrix<double> >(-1, temp));
    }
}

void KdCMVN::CacheFrame(int32 frame, const MatrixBase<double> &stats)
{
    KALDI_ASSERT(frame >= 0);
    if (frame % opts_.modulus == 0)
    {
        int32 n = frame / opts_.modulus;
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
            int32 index = frame % cached_stats_ring_.size();
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

void KdCMVN::ComputeStatsForFrame(int32 frame,
                                      MatrixBase<double> *stats_out)
{
    KALDI_ASSERT(frame >= 0 && frame < src_->NumFramesReady());

    int32 dim = this->Dim(), cur_frame;
    GetMostRecentCachedFrame(frame, &cur_frame, stats_out);

    Vector<BaseFloat> &feats(temp_feats_);
    Vector<double> &feats_dbl(temp_feats_dbl_);
    while (cur_frame < frame)
    {
        cur_frame++;
        src_->GetFrame(cur_frame, &feats);
        feats_dbl.CopyFromVec(feats);
        stats_out->Row(0).Range(0, dim).AddVec(1.0, feats_dbl);
        if (opts_.normalize_variance)
            stats_out->Row(1).Range(0, dim).AddVec2(1.0, feats_dbl);
        (*stats_out)(0, dim) += 1.0;
        // it's a sliding buffer; a frame at the back may be
        // leaving the buffer so we have to subtract that.
        int32 prev_frame = cur_frame - opts_.cmn_window;
        if (prev_frame >= 0)
        {
            // we need to subtract frame prev_f from the stats.
            src_->GetFrame(prev_frame, &feats);
            feats_dbl.CopyFromVec(feats);
            stats_out->Row(0).Range(0, dim).AddVec(-1.0, feats_dbl);
            if (opts_.normalize_variance)
                stats_out->Row(1).Range(0, dim).AddVec2(-1.0, feats_dbl);
            (*stats_out)(0, dim) -= 1.0;
        }
        CacheFrame(cur_frame, (*stats_out));
    }
}


// static
void OnlineCmvn::SmoothOnlineCmvnStats(const MatrixBase<double> &speaker_stats,
                                       const MatrixBase<double> &global_stats,
                                       const OnlineCmvnOptions &opts,
                                       MatrixBase<double> *stats) {
  if (speaker_stats.NumRows() == 2 && !opts.normalize_variance)
  {
    // this is just for efficiency: don't operate on the variance if it's not
    // needed.
    int32 cols = speaker_stats.NumCols();  // dim + 1
    SubMatrix<double> stats_temp(*stats, 0, 1, 0, cols);
    SmoothOnlineCmvnStats(speaker_stats.RowRange(0, 1),
                          global_stats.RowRange(0, 1),
                          opts, &stats_temp);
    return;
  }
  int32 dim = stats->NumCols() - 1;
  double cur_count = (*stats)(0, dim);
  // If count exceeded cmn_window it would be an error in how "window_stats"
  // was accumulated.
  KALDI_ASSERT(cur_count <= 1.001 * opts.cmn_window);
  if (cur_count >= opts.cmn_window)
    return;
  if (speaker_stats.NumRows() != 0)
  {  // if we have speaker stats..
    double count_from_speaker = opts.cmn_window - cur_count;
    double speaker_count = speaker_stats(0, dim);
    if (count_from_speaker > opts.speaker_frames)
      count_from_speaker = opts.speaker_frames;
    if (count_from_speaker > speaker_count)
      count_from_speaker = speaker_count;
    if (count_from_speaker > 0.0)
      stats->AddMat(count_from_speaker / speaker_count,
                             speaker_stats);
    cur_count = (*stats)(0, dim);
  }
  if (cur_count >= opts.cmn_window)
    return;
  if (global_stats.NumRows() != 0)
  {
    double count_from_global = opts.cmn_window - cur_count,
        global_count = global_stats(0, dim);
    KALDI_ASSERT(global_count > 0.0);
    if (count_from_global > opts.global_frames)
      count_from_global = opts.global_frames;
    if (count_from_global > 0.0)
      stats->AddMat(count_from_global / global_count,
                    global_stats);
  }
  else
  {
    KALDI_ERR << "Global CMN stats are required";
  }
}

void KdCMVN::GetFrame(int32 frame,
                      VectorBase<BaseFloat> *feat)
{
    src_->GetFrame(frame, feat);
    KALDI_ASSERT(feat->Dim() == this->Dim());
    int32 dim = feat->Dim();
    Matrix<double> &stats(temp_stats_);
    stats.Resize(2, dim + 1, kUndefined);  // Will do nothing if size was correct.
    if( frozen_state_.NumRows()!=0 )
    {  // the CMVN state has been frozen.
        stats.CopyFromMat(frozen_state_);
    }
    else
    {
        // first get the raw CMVN stats (this involves caching..)
        this->ComputeStatsForFrame(frame, &stats);
        // now smooth them.
        SmoothOnlineCmvnStats(orig_state_.speaker_cmvn_stats,
                              orig_state_.global_cmvn_stats,
                              opts_,
                              &stats);
    }

    if (!skip_dims_.empty())
    {
        FakeStatsForSomeDims(skip_dims_, &stats);
    }

    // call the function ApplyCmvn declared in ../transform/cmvn.h, which
    // requires a matrix.
    // 1 row; num-cols == dim; stride  == dim.
    SubMatrix<BaseFloat> feat_mat(feat->Data(), 1, dim, dim);
    // the function ApplyCmvn takes a matrix, so form a one-row matrix to give it.
    if (opts_.normalize_mean)
    {
        ApplyCmvn(stats, opts_.normalize_variance, &feat_mat);
    }
    else
    {
        KALDI_ASSERT(!opts_.normalize_variance);
    }
}

void KdCMVN::Freeze(int32 cur_frame)
{
    int32 dim = this->Dim();
    Matrix<double> stats(2, dim + 1);
    // get the raw CMVN stats
    this->ComputeStatsForFrame(cur_frame, &stats);
    // now smooth them.
    SmoothOnlineCmvnStats(orig_state_.speaker_cmvn_stats,
                          orig_state_.global_cmvn_stats,
                          opts_,
                          &stats);
    this->frozen_state_ = stats;
}

void KdCMVN::GetState(int32 cur_frame,
                      OnlineCmvnState *state_out)
{
    *state_out = this->orig_state_;
    { // This block updates state_out->speaker_cmvn_stats
        int32 dim = this->Dim();
        if (state_out->speaker_cmvn_stats.NumRows() == 0)
            state_out->speaker_cmvn_stats.Resize(2, dim + 1);
        Vector<BaseFloat> feat(dim);
        Vector<double> feat_dbl(dim);
        for (int32 t = 0; t <= cur_frame; t++) {
            src_->GetFrame(t, &feat);
            feat_dbl.CopyFromVec(feat);
            state_out->speaker_cmvn_stats(0, dim) += 1.0;
            state_out->speaker_cmvn_stats.Row(0).Range(0, dim).AddVec(1.0, feat_dbl);
            state_out->speaker_cmvn_stats.Row(1).Range(0, dim).AddVec2(1.0, feat_dbl);
        }
    }
    // Store any frozen state (the effect of the user possibly
    // having called Freeze().
    state_out->frozen_state = frozen_state_;
}

void KdCMVN::SetState(OnlineCmvnState cmvn_state)
{
    KALDI_ASSERT(cached_stats_modulo_.empty() &&
                 "You cannot call SetState() after processing data.");
    orig_state_ = cmvn_state;
    frozen_state_ = cmvn_state.frozen_state;
}
