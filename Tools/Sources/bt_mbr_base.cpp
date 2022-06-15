#include "bt_mbr_base.h"

void bt_writeBarResult(QVector<BtWord> result)
{
    QFile bar_file(BT_BAR_RESULT);

    if( !bar_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << BT_BAR_RESULT;
        return;
    }

    QTextStream out(&bar_file);

    for( int i=0 ; i<result.length() ; i++ )
    {
        if( result[i].is_final )
        {
            out << "%{F#ddd}";
        }
        else
        {
            out << "%{F#777}";
        }

        if( result[i].conf<KAL_HARD_TRESHOLD )
        {
            out << "%{u#f00}%{+u}";
        }
        else if( result[i].conf==1.00 )
        {
            out << "%{u#1d1}%{+u}";
        }
        else if( result[i].conf>KAL_CONF_TRESHOLD )
        {
            out << "%{u#16A1CF}%{+u}";
        }
        else
        {
            out << "%{u#CF8516}%{+u}";
        }
        out << result[i].word;

        out << "%{-u} ";
    }
    out << "\n";

    bar_file.close();
}

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
