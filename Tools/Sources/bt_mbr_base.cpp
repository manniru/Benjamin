#include "bt_mbr_base.h"

// Not called in windows (only in test mode)
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
            out << "%{U#f00}%{+U}";
        }
        else if( result[i].conf==1.00 )
        {
            out << "%{U#1d1}%{+U}";
        }
        else if( result[i].conf>KAL_CONF_TRESHOLD )
        {
            out << "%{U#16A1CF}%{+U}";
        }
        else
        {
            out << "%{U#CF8516}%{+U}";
        }
        out << result[i].word;

        out << "%{-U} ";
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
