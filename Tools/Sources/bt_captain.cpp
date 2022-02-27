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

    start_treshold = -BT_HISTORY_SIZE/1000.0;
}

void BtCaptain::parse(QVector<BtWord> in_words)
{
    if( in_words.empty() )
    {
        return;
    }

    utterance = "";
    QString cmd = KAL_SI_DIR"main.sh \"";

    for( int i=0 ; i<in_words.length() ; i++ )
    {
        addWord(in_words[i]);

        if( in_words[i].is_final )
        {
            cmd += in_words[i].word;
            cmd += " ";
        }
    }

    cmd += "\"";
    system(cmd.toStdString().c_str());
    //    qDebug() << "exec" << cmd;

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
                    qDebug() << "addWord" << word.word << word.is_final;
    // change not final last to final
    // remove all that are not final
    if( history.length() )
    {
        BtWord last = history.last();

        if( (last.is_final==false) &&
            (last.word==word.word) &&
            (word.is_final) )
        {
            qDebug() << "word" << last.word;
            history.last().is_final = true;
            return;
        }

        while( history.last().is_final==false )
        {
            history.removeLast();

            if( history.isEmpty() )
            {
                break;
            }
        }

    }
    history.append(word);
}

void BtCaptain::shiftHistory()
{
    start_treshold += BT_HISTORY_UPDATE/1000.0;

    for( int i=0 ; i<history.length() ; i++ )
    {
        if( history[i].time<start_treshold ) //5 second history size
        {
            history.remove(i);
            i--;
        }
    }

    bt_writeBarResult(history);
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
