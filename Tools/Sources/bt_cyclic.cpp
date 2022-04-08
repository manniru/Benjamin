#include "bt_cyclic.h"
#include <QDebug>

BtCyclic::BtCyclic(int size, QObject *parent) : QObject(parent)
{
    buffer = (int16_t *)malloc( sizeof(int16_t)*size );
    buff_size = size;

    read_p = 0;
    write_p = 0;

    for( int i=0 ; i<buff_size ; i++ )
    {
        buffer[i] = 0;
    }
}

// return negative on erorr
// use "fake" to read without changing read_p to be used
// in multicore mode
int BtCyclic::read(int16_t *data, int size, int *fake)
{
    int num = 0;
    int read_pf = read_p;//fake read pointer

    if( fake!=NULL )
    {
        read_pf = *fake;
    }

    if( size>getDataSize() )
    {
        size = getDataSize();
    }

    for( int i=0 ; i<size ; i++ )
    {
        data[i] = buffer[read_pf];
        read_pf++;

        if( read_pf>buff_size)
        {
            read_pf = 0;
        }
        num++;
    }

    if( fake==NULL )
    {
        read_p = read_pf;
    }
    else
    {
        *fake = read_pf;
    }

    return num;
//    qDebug() << "raad bds" << buff_data_size/BT_REC_RATE << size;
}

int BtCyclic::read(float *data, int size, int *fake)
{
    int num = 0;
    int read_pf = read_p;//fake read pointer

    if( fake!=NULL )
    {
        read_pf = *fake;
    }

    if( size>getDataSize() )
    {
        size = getDataSize();
    }

    for( int i=0 ; i<size ; i++ )
    {
        data[i] = buffer[read_pf];
        read_pf++;

        if( read_pf>buff_size)
        {
            read_pf = 0;
        }
        num++;
    }

    if( fake==NULL )
    {
        read_p = read_pf;
    }
    else
    {
        *fake = read_pf;
    }

    return num;
//    qDebug() << "raad bds" << buff_data_size/BT_REC_RATE << size;
}

// return negative on erorr
int BtCyclic::read(kaldi::Vector<float> *data, int size)
{
    int num = 0;

    if( size>getDataSize() )
    {
        size = getDataSize();
    }

    for( int i=0 ; i<size ; i++ )
    {
        (*data)(i) = buffer[read_p];
        read_p++;

        if( read_p>buff_size)
        {
            read_p = 0;
        }
        num++;
    }

//    qDebug() << "raad" << size << "bds"
//             << getDataSize();
    return num;
}

// return negative on erorr
void BtCyclic::write(int16_t *data, int size)
{
    for( int i=0 ; i<size ; i++ )
    {
        buffer[write_p] = data[i];
        write_p++;

        if( write_p>=buff_size )
        {
            write_p = 0;
        }
    }
//    qDebug() << "wrte bds" << buff_data_size/BT_REC_RATE << size;
}

// return negative on erorr
void BtCyclic::write(QVector<int16_t> *data)
{
    int max_len = data->size();
    for( int i=0 ; i<max_len ; i++ )
    {
        buffer[write_p] = (*data)[i];
        write_p++;

        if( write_p>=buff_size )
        {
            write_p = 0;
        }
    }
}

// return negative on erorr
void BtCyclic::constWrite(int16_t data, int size)
{
    for( int i=0 ; i<size ; i++ )
    {
        buffer[write_p] = data;
        write_p++;

        if( write_p>=buff_size )
        {
            write_p = 0;
        }
    }
//    qDebug() << "cWrt bds" << buff_data_size/BT_REC_RATE << size;
}

// get read pointer back
void BtCyclic::rewind(int count)
{
    if( count>buff_size )
    {
        qDebug() << "wrong usage of BtCyclic::rewind function";
    }

    read_p = read_p-count;

    if( read_p<0 )
    {
        read_p = read_p+buff_size;
    }

    //    qDebug() << "buff_data_size" << buff_data_size/BT_REC_RATE;
}

int BtCyclic::getDataSize(int *fake)
{
    int read_pf = read_p;//fake read pointer

    if( fake!=NULL )
    {
        read_pf = *fake;
    }

    if( read_pf<write_p )
    {
        return write_p-read_pf;
    }
    else if( read_pf==write_p )
    {
        return 0;
    }
    else
    {
        return (buff_size-read_pf) + write_p;
    }
}

int BtCyclic::getFreeSize()
{
    return buff_size-getDataSize();
}
