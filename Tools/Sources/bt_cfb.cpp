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

void BtCFB::writeFeat(uint frame, Vector<float> *out)
{
    int index = frame%len;
    BtFrameBuf *buf = &data[index];

    for( int i=0 ; i<BT_DELTA_SIZE ; i++ )
    {
        (*out)(i) = buf->delta[i];
    }
}
