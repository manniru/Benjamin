#include "bt_cyclic.h"
#include <QDebug>

BtCyclic::BtCyclic(int size, QObject *parent) : QObject(parent)
{
    buffer = malloc( sizeof(int16_t)*size );
    buff_size = size;

    read_p = 0;
    write_p = 0;
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

    return 0;
}

// return negative on erorr
void BtCyclic::write(int16_t *data, int size)
{
    for( int i=0 ; i<size ; i++ )
    {
        buffer[write_p] = data[i];
        write_p++;

        if( write_p>buff_size)
        {
            write_p = 0;
        }
    }
}
