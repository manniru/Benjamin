#include "bt_cfb.h"
#include <QDebug>

using namespace kaldi;

BtCFB::BtCFB(int size, QObject *parent) : QObject(parent)
{
    data = (BtFrameBuf *)malloc( sizeof(BtFrameBuf)*size );
    len = size;
}

BtFrameBuf *BtCFB::get(uint frame)
{
    int index = frame%len;
    return &data[index];
}

void BtCFB::writeVec(uint frame, Vector<float> *vec)
{
    int index = frame%len;
    BtFrameBuf *buf = &data[index];

    for( int i=0 ; i<BT_FEAT_SIZE ; i++ )
    {
        buf->ceps[i] = (*vec)(i);
        buf->cmvn[i] = (*vec)(i); // fill both, cmvn val adds later
        buf->have_cmvn = false;
    }
}

void BtCFB::writeFeat(uint frame, Vector<float> *out)
{
    int index = frame%len;
    BtFrameBuf *buf = &data[index];

    for( int i=0 ; i<BT_DELTA_SIZE ; i++ )
    {
        (*out)(i) = buf->delta[i];
    }
}
