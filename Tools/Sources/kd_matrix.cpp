#include "kd_matrix.h"
#include <QDebug>

KdMatrix::KdMatrix()
{
    d = NULL;
    rows = 0;
    cols = 0;
}

KdMatrix::~KdMatrix()
{
//    free();
}

void KdMatrix::free()
{
    if( d )
    {
        for( int i=0 ; i<rows ; i++ )
        {
            delete d[i];
        }
        delete d;
        d = NULL;
        rows = 0;
        cols = 0;
    }
}

void KdMatrix::resize(int row, int column)
{
    free();

    d = (float **)malloc(sizeof(float *)*row);
    for(  int i=0 ; i<row ; i++ )
    {
        d[i] = (float *)malloc(sizeof(float)*column);
    }
    rows = row;
    cols = column;
}

void KdMatrix::setZero()
{
    for(  int i=0 ; i<rows ; i++ )
    {
        for( int j=0 ; j<cols ; j++ )
        {
            d[i][j] = 0;
        }
    }
}

