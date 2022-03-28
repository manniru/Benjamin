#include "kd_matrix_dbg.h"
#include <QDebug>

using namespace kaldi;

// only apply mean
void kd_printMat(Matrix<double> stats)
{
    QString buf;
    qDebug() << "START";
    for( int i=0 ; i<stats.NumRows() ; i++ )
    {
        buf = "";
        for( int j=0 ; j<stats.NumCols() ; j++ )
        {
            buf += QString::number(stats.Row(i).Data()[j]);
            buf += " ";
        }
        qDebug() << buf;
    }
}

// only apply mean
void kd_printMat2(MatrixBase<float> *stats)
{
    QString buf;
    qDebug() << "START";
    for( int i=0 ; i<stats->NumRows() ; i++ )
    {
        buf = "";
        for( int j=0 ; j<stats->NumCols() ; j++ )
        {
            buf += QString::number(stats->Row(i).Data()[j]);
            buf += " ";
        }
        qDebug() << buf;
    }
}

