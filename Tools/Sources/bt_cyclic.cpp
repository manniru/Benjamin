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
void BtCyclic::raad(int16_t *data, int size)
{
    for( int i=0 ; i<size ; i++ )
    {
        data[i] = buffer[read_p];
        read_p++;

        if( read_p>buff_size)
        {
            read_p = 0;
        }
    }

    buff_data_size -= size;
//    qDebug() << "raad bds" << buff_data_size/BT_REC_RATE << size;
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
    buff_data_size += size;
//    qDebug() << "wrte bds" << buff_data_size/BT_REC_RATE << size;
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
    buff_data_size += size;
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

    buff_data_size += count;
//    qDebug() << "buff_data_size" << buff_data_size/BT_REC_RATE;
}
