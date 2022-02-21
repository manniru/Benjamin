#include "bt_captain.h"
#include <QDebug>

BtCaptain::BtCaptain(QObject *parent) : QObject(parent)
{
    setbuf(stdout,NULL);
//    printf("%s-%.2f(%.2f->%.2f) ", word.word.toStdString().c_str()
//           , word.conf, word.start, word.end);
//    printf("raw conf: %s ", BT_TIME_NOW.toStdString().c_str());

    time_shifter = new QTimer;
    connect(time_shifter, SIGNAL(timeout()),
            this, SLOT(shiftHistory()));
    time_shifter->start(BT_HISTORY_UPDATE);

    start_treshold = 0;
}

void BtCaptain::parse(QVector<BtWord> in_words)
{
    utterance = "";

    for( int i=0 ; i<in_words.length() ; i++ )
    {
        addWord(in_words[i]);
    }

    bt_writeBarResult(history);
}

void BtCaptain::printWords(QString words)
{
    QStringList word_list = words.split(" ");

    for( int i=1 ; i<word_list.size() ; i++ )
    {
        printf("%-10s ", word_list[i].toStdString().c_str());
    }
    printf("\n");
}

void BtCaptain::addWord(BtWord word)
{
    history.append(word);
}

void BtCaptain::shiftHistory()
{
    start_treshold += BT_HISTORY_UPDATE/1000;

    for( int i=0 ; i<history.length() ; i++ )
    {
        if( history[i].time<start_treshold ) //5 second history size
        {
            history.remove(i);
            i--;
        }
    }
}

void bt_writeBarResult(QVector<BtWord> result)
{
    QFile bar_file(BT_BAR_RESULT);

    if (!bar_file.open(QIODevice::WriteOnly | QIODevice::Text))
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
