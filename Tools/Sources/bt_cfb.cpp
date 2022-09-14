#include "bt_cfb.h"
#include <QDebug>

BtCFB::BtCFB(int size, QObject *parent) : QObject(parent)
{
    data = (BtFrameBuf *)malloc( sizeof(BtFrameBuf)*size );
    len = size; 
}

BtCFB::~BtCFB()
{
    free(data);
}

BtFrameBuf *BtCFB::get(uint frame)
{
    int index = frame%len;
    return &data[index];
}
