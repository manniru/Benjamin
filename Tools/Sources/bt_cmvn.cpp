#include "bt_cmvn.h"
#include <QDebug>
#include <QFile>
#include <Qt>

#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <matrix/kaldi-matrix.h>

using namespace kaldi;

BtCMVN::BtCMVN(BtCFB *feat)
{
    i_feature = feat;
    resetSum();
    readGlobal();
}

void BtCMVN::resetSum()
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

BtCMVN::~BtCMVN()
{
}

void BtCMVN::updateStats(BtFrameBuf *buf)
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
void BtCMVN::computeFinalStats()
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
void BtCMVN::calc(uint frame)
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

void BtCMVN::readGlobal()
{
    QFile file(BT_GCMVN_PATH);

    if( !file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << BT_GCMVN_PATH;
        return;
    }

    file.readLine(); //skip first line
    QString line = file.readLine();
    QStringList val = line.split(" ", QString::SkipEmptyParts);

    if( val.size()!=(BT_FEAT_SIZE+2) )
    {
        qDebug() << "Error 122: GVMN is wrong in size" << val.size();
        exit(1);
    }

    bool ok;
    for( int i=0 ; i<(BT_FEAT_SIZE+1) ; i++ )
    {
        global_state[i] = val[i].toDouble(&ok);
        if( !ok )
        {
            qDebug() << "Error 123: Error converting val in GCMVN" << val[i] << i;
            exit(1);
        }
    }

    file.close();
}
