#include "bt_cmvn.h"
#include <QDebug>

#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <matrix/kaldi-matrix.h>

using namespace kaldi;

KdCMVN::KdCMVN(kaldi::Matrix<float> g_state, BtCFB *feat)
{
    i_feature = feat;
    resetSum();

    for( int i=0 ; i<BT_FEAT_SIZE+1 ; i++ )
    {
        global_state[i] = g_state(0, i);
    }
}

void KdCMVN::resetSum()
{
    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        f_sum[i] = 0;
        full_sum[i] = 0;
    }

    feature_buf.head = 0;
    feature_buf.tail = 0;
    feature_buf.len  = 0;
}

KdCMVN::~KdCMVN()
{
}

void KdCMVN::updateStats(BtFrameBuf *buf)
{
    if( buf->have_cmvn )
    {
        return;
    }

    int head = feature_buf.head;
    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        feature_buf.data[head][i]  = buf->ceps[i];
        f_sum[i]             += buf->ceps[i];
    }

    feature_buf.head++;
    if( feature_buf.head>=BT_CMVN_WINDOW )
    {
        feature_buf.head = 0;
    }

    feature_buf.len++;
    if( feature_buf.len>BT_CMVN_WINDOW ) //need to remove element from window
    {
        int tail = feature_buf.tail;
        for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
        {
            f_sum[i] -= feature_buf.data[tail][i];
        }

        feature_buf.tail++;
        if( feature_buf.tail>=BT_CMVN_WINDOW )
        {
            feature_buf.tail = 0;
        }
        feature_buf.len = BT_CMVN_WINDOW;
    }
}

// Add from global CMVN if no frame is in stat
void KdCMVN::computeFinalStats()
{
    double remain_f = BT_CMVN_WINDOW - feature_buf.len; //number of frame needed
    double global_N = global_state[BT_FEAT_SIZE];

    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        full_sum[i] = f_sum[i] + global_state[i]*remain_f/global_N;
    }
//    stats->AddMat( remain_f/global_N, global_state);
}

//called from outside
void KdCMVN::calc(int frame)
{
    BtFrameBuf *buf = i_feature->get(frame);
    updateStats(buf);
    computeFinalStats();

    if( buf->have_cmvn )
    {
        return;
    }

    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        buf->cmvn[i] -= full_sum[i]/BT_CMVN_WINDOW;
        buf->delta[i] = buf->cmvn[i];
    }

    buf->have_cmvn = true;
}
