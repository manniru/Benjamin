#include "bt_mbr_base.h"
#include <QDebug>

void bt_printResult(QVector<BtWord> result)
{
    QString buf;

    for( int i=0 ; i<result.length() ; i++ )
    {
        buf += result[i].word;
        buf += "(";
        buf += QString::number(result[i].time);
        buf += ",";
        buf += QString::number(result[i].conf);
        buf += ")";
        buf += " ";
    }

    qDebug() << buf;
}
