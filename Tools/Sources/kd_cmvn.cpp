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
    global_state = cmvn_state;
    Dim = dim;
}

int KdCMVN::getLastCachedFrame(int frame, MatrixBase<double> *stats)
{
    int cached_frame;
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
            stats->CopyFromMat(cached_stats_ring_[index].second);
            return t;
        }
    }
    int n = frame / modulus;
    if( n>=cached_stats_modulo_.size() )
    {
        if( cached_stats_modulo_.size()==0 )
        {
            stats->SetZero();
            return -1;
        }
        else
        {
            n = static_cast<int>(cached_stats_modulo_.size() - 1);
        }
    }
    cached_frame = n * modulus;
    KALDI_ASSERT(cached_stats_modulo_[n] != NULL);
    stats->CopyFromMat(*(cached_stats_modulo_[n]));
    return cached_frame;
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

void KdCMVN::ComputeStats(int frame,
                          KdRecyclingVector *o_features,
                          MatrixBase<double> *stats_out)
{
    KALDI_ASSERT(frame >= 0 && frame < o_features->Size());

    int cur_frame;
    cur_frame = getLastCachedFrame(frame, stats_out);

    while( cur_frame<frame )
    {
        cur_frame++;
        temp_feats_.CopyFromVec(*(o_features->At(cur_frame)));////GET FRAME
        temp_feats_dbl_.CopyFromVec(temp_feats_);
        stats_out->Row(0).Range(0, Dim).AddVec(1.0, temp_feats_dbl_);
        (*stats_out)(0, Dim) += 1.0;
        // it's a sliding buffer; a frame at the back may be
        // leaving the buffer so we have to subtract that.
        int prev_frame = cur_frame - cmn_window;
        if( prev_frame >= 0)
        {
            // we need to subtract frame prev_f from the stats.
            temp_feats_.CopyFromVec(*(o_features->At(prev_frame)));////GET FRAME
            temp_feats_dbl_.CopyFromVec(temp_feats_);
            stats_out->Row(0).Range(0, Dim).AddVec(-1.0, temp_feats_dbl_);
            (*stats_out)(0, Dim) -= 1.0;
        }
        CacheFrame(cur_frame, (*stats_out));
    }
}

// SmoothOnlineCmvnStats
void KdCMVN::smoothStats(MatrixBase<double> *stats)
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

    double count_from_global = cmn_window - cur_count;
    double global_dim = global_state(0, dim);
    if( count_from_global>global_frames )
    {
        count_from_global = global_frames;
    }
    if( count_from_global>0 )
    {
        stats->AddMat(count_from_global / global_dim,
                      global_state);
    }
}

//called from outside
void KdCMVN::GetFrame(int frame,
                      KdRecyclingVector *o_features,
                      VectorBase<float> *feat)
{
    KALDI_ASSERT(feat->Dim() == Dim); // = 13
    Matrix<double> stats(temp_stats_);
    stats.Resize(2, Dim + 1, kUndefined);  // Will do nothing if size was correct.
    ComputeStats(frame, o_features, &stats);
//    smoothStats(&stats);

    // ApplyCmvn
    double N = -global_state(0, Dim); //count

    for( int i=0 ; i<Dim ; i++ )
    {
//        qDebug() << i << global_state(0, i)/N;
        (*feat)(i) += global_state(0, i)/N;
    }
}
