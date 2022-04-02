#include "kd_cmvn.h"
#include <QDebug>

#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <matrix/kaldi-matrix.h>

using namespace kaldi;

KdCMVN::KdCMVN(kaldi::Matrix<float> g_state, BtCFB *feat)
{
    temp_stats_.Resize(2, BT_FEAT_SIZE + 1);
    global_state = g_state;
    Dim = BT_FEAT_SIZE;
    feature = feat;
}

// go back in time and find if a frame is cached
int KdCMVN::getLastCached(int frame, MatrixBase<double> *stats)
{
    int cached_frame;
    InitRingBuffer();

    for( int i=frame ; i>=0 && i>=(frame-ring_size) ; i-- )
    {
        if( i%modulus==0 )
        {
            // this frame cached in cached_stats_modulo_
            break;
        }
        int index = i % ring_size;
        if( cached_stats[index].first == i)
        {
            stats->CopyFromMat(cached_stats[index].second);
            return i;
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
    if( cached_stats.empty() )
    {
        Matrix<double> temp(2, Dim + 1);
        cached_stats.resize(ring_size,
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
        if( !cached_stats.empty())
        {
            int index = frame % cached_stats.size();
            cached_stats[index].first = frame;
            cached_stats[index].second.CopyFromMat(stats);
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

void KdCMVN::updateStats(int frame,
                          KdRecyclingVector *o_features,
                          MatrixBase<double> *stats_out)
{
    kaldi::Vector<float>  temp_feats_;
    kaldi::Vector<double> temp_feats_dbl_;
    temp_feats_dbl_.Resize(Dim);
    temp_feats_.Resize(Dim);
    int last_f = getLastCached(frame, stats_out);

    for( int i=last_f+1 ; i<frame ; i++ )
    {
        temp_feats_.CopyFromVec(*(o_features->At(i)));////GET FRAME
        temp_feats_dbl_.CopyFromVec(temp_feats_);
        stats_out->Row(0).Range(0, Dim).AddVec(1.0, temp_feats_dbl_);
        (*stats_out)(0, Dim) += 1.0;
        // it's a sliding buffer; a frame at the back may be
        // leaving the buffer so we have to subtract that.
        int prev_frame = i - cmn_window;
        if( prev_frame>=0 )
        {
            // we need to subtract frame prev_f from the stats.
            temp_feats_.CopyFromVec(*(o_features->At(prev_frame)));////GET FRAME
            temp_feats_dbl_.CopyFromVec(temp_feats_);
            stats_out->Row(0).Range(0, Dim).AddVec(-1.0, temp_feats_dbl_);
            (*stats_out)(0, Dim) -= 1.0;
        }
        CacheFrame(i, (*stats_out));
    }
}

// Add from global CMVN if no frame is in stat
void KdCMVN::addGlobal(MatrixBase<double> *stats)
{
    int last_i = stats->NumCols() - 1;
    double count = (*stats)(0, last_i);
    if( count >= cmn_window)
    {
        // already have enough data
        return;
    }

    double remain_f = cmn_window - count; //number of frame needed
    double global_N = global_state(0, last_i);
//    stats->AddMat( remain_f/global_N, global_state);
}

//called from outside
void KdCMVN::calc(int frame)
{
    Matrix<double> stats(temp_stats_);
    stats.Resize(2, Dim + 1, kUndefined);  // Will do nothing if size was correct.
//    updateStats(frame, o_features, &stats);
//    addGlobal(&stats);

    // ApplyCmvn
    double N = -global_state(0, Dim); //count
    BtFrameBuf *buf = feature->get(frame);

    if( buf->have_cmvn )
    {
        return;
    }

    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
//        qDebug() << i << global_state(0, i)/N;
        buf->cmvn[i] += global_state(0, i)/N;
        buf->delta[i] = buf->cmvn[i];
    }

    buf->have_cmvn = true;
}
