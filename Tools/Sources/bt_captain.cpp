#include "bt_captain.h"
#include <QDebug>

BtCaptain::BtCaptain(QObject *parent) : QObject(parent)
{
    setbuf(stdout,NULL);

    lastword.word = "";
}

void BtCaptain::parse(QVector<BtWord> in_words)
{
    words.clear();
    shiftHistory();
    utterance = "";

//    if( in_words.length() )
//    {
//        printConf(in_words);
//    }

    int index_LastWord = lastWordIndex(in_words);

    if( index_LastWord )
    {
        in_words.remove(0, index_LastWord);
    }

    for( int i=0 ; i<in_words.length() ; i++ )
    {
        processUtterance(in_words[i]);
    }

    if( words.length() )
    {
        printf("\n");
        lastword = words.last();
    }
    else
    {
        if( lastword.word!="" )
        {
            lastword.time = lastword.time-BT_DEC_TIMEOUT;

            if( lastword.time<0.5 )
            {
                qDebug() << "word removed" << lastword.word;
                lastword.word = "";
            }
        }
    }

    bt_writeBarResult(history);
}

void BtCaptain::processUtterance(BtWord word)
{
    if( isValidTime(word) )
    {
        addWord(word);
        printf("%s-%.2f(%.2f->%.2f) ", word.word.toStdString().c_str()
               , word.conf, word.start, word.end);
    }
}

bool BtCaptain::isValidUtterance()
{
    if( utterance.isEmpty() )
    {
        return false;
    }
    if( getAvgDetection()<KAL_UDET_TRESHOLD && getAvgConfidence()<KAL_UCON_TRESHOLD )
    {
        qDebug() << "Unvalid" << getAvgDetection() << getAvgConfidence() << history.length();
        return false;
    }

    return true;
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

QString BtCaptain::getUtterance()
{
    return utterance;
}

bool BtCaptain::isValidTime(BtWord word)
{
    if( word.time<0.7 )
    {
        return false;
    }

    if( (word.time>=BT_REC_SIZE-0.8) )
    {
        return false;
    }

    if( word.end>BT_REC_SIZE-0.4 )
    {
        return false;
    }

    return true;
}

void BtCaptain::addWord(BtWord word)
{
    BtWord word_buf = word;
    if( word.conf>KAL_HARD_TRESHOLD )
    {
        if( utterance.length() )
        {
            utterance += " ";
        }
        utterance += word.word;

        word_buf.time = word.time-BT_DEC_TIMEOUT;
        words.append(word_buf);
    }

    history.append(word);
}

int BtCaptain::lastWordIndex(QVector<BtWord> in_words)
{
    if ( lastword.word.isEmpty() )
    {
        return 0;
    }

    int index = lastWordIndex(-0.1, 0.1, in_words);

    if( index==0 )
    {
        index = lastWordIndex(-0.2, 0, in_words);
    }
    if( index==0 )
    {
        qDebug() << BT_TIME_NOW << "Lastword not found, max_dist->-0.5";
        index = lastWordIndex(-0.5, 0, in_words);
    }
    if( index==0 )
    {
        qDebug() << BT_TIME_NOW << "Lastword not found, max_dist->+0.2";
        index = lastWordIndex(0, 0.2, in_words);
    }
    if( index==0 )
    {
        qDebug() << BT_TIME_NOW << "Lastword not found, max_dist->+0.5";
        index = lastWordIndex(0, 0.5, in_words);
    }
    if( index==0 )
    {
        qDebug() << BT_TIME_NOW << "!!!!!!Last Word Not Found" << lastword.word << lastword.time;
    }

    return index;
}

//Return first word that match the timing
//Being first word is a designed feature
int BtCaptain::lastWordIndex(double min, double max, QVector<BtWord> in_words)
{
    for( int i=0 ; i<in_words.length() ; i++ )
    {
        if( in_words[i].word==lastword.word )
        {
            double dist = in_words[i].time-lastword.time;
            if( min<=dist && dist<=max )
            {
//                qDebug() << "dist" << dist;
                return i+1;
            }
        }
    }

    return 0;
}

void BtCaptain::printConf(QVector<BtWord> in_words)
{
    printf("raw conf: %s ", BT_TIME_NOW.toStdString().c_str());

    for( int i=0 ; i<in_words.length() ; i++ )
    {
        printf("%s(%f) ", in_words[i].word.toStdString().c_str(), in_words[i].time);
    }

    printf("\n");
}

void BtCaptain::shiftHistory()
{
    for( int i=0 ; i<history.length() ; i++ )
    {
        if( history[i].time<(BT_REC_SIZE-5) ) //5 second history size
        {
            history.remove(i);
            i--;
        }
        else
        {
            history[i].time = history[i].time-BT_DEC_TIMEOUT;
        }
    }
}

double BtCaptain::getAvgConfidence()
{
    double sum = 0;
    for( int i=0 ; i<history.length() ; i++ )
    {
        sum += history[i].conf;
    }

    return sum/history.length();
}

double BtCaptain::getAvgDetection()
{
    double detection = 0;
    for( int i=0 ; i<history.length() ; i++ )
    {
        if( history[i].conf==1.00 && history[i].time>0 )
        {
            detection += 2;
        }
        else if( history[i].conf>KAL_CONF_TRESHOLD )
        {
            detection += 1;
        }
        else if( history[i].word=="up" || history[i].word=="two" )
        {
            if( history[i].conf>KAL_HARD_TRESHOLD )
            {
                detection += 1;
            }
        }
        else if( history[i].conf<KAL_HARD_TRESHOLD )
        {
            detection -= 1;
        }
    }

    return detection/history.length();
}

void bt_writeBarResult(QVector<QString> result)
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
        out << "%{u#1d1}%{+u}";
        out << result[i];

        out << "%{-u} ";
    }
    out << "\n";

    bar_file.close();
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
        if( result[i].conf<KAL_CONF_TRESHOLD )
        {
            out << "%{F#777}";
        }
        else
        {
            out << "%{F#ddd}";
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
