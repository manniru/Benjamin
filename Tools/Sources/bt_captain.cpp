#include "bt_captain.h"
#include <QDebug>

BtCaptain::BtCaptain(QObject *parent) : QObject(parent)
{
    setbuf(stdout,NULL);
    conf = new BtConfidence;
}

void BtCaptain::parse()
{
    if( words.length() )
    {
        lastword = words.last();
    }
    else
    {
        lastword.word = "";
    }
    words.clear();
    shiftHistory();
    utterance = "";

    ///FIXME: get conf
    conf->parseConfidence();

    ///FIXME: find last word time
    int index_LastWord = lastWordIndex();
    conf->words.remove(index_LastWord);

    for( int i=0 ; i<conf->words.length() ; i++ )
    {
        processUtterance(conf->words[i]);
    }

    ///FIXME: if any output!
    if( words.length() )
    {
        printf("\n");
    }

    writeBarResult();
}

void BtCaptain::processUtterance(BtWord word)
{
    if( isValidTime(word.time) )
    {
        addWord(word.word, word.time, word.conf);
        ///FIXME: Add start and end to BtWord
//        printf("%s-%.2f(%.2f->%.2f) ", word.word.toStdString().c_str()
//               , conf, start, end);
    }
}

void BtCaptain::writeBarResult()
{
    QFile bar_file(BT_BAR_RESULT);

    if (!bar_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << BT_BAR_RESULT;
        return;
    }

    QTextStream out(&bar_file);

    for( int i=0 ; i<history.length() ; i++ )
    {
        if( history[i].conf<KAL_CONF_TRESHOLD )
        {
            out << "%{F#777}";
        }
        else
        {
            out << "%{F#ddd}";
        }

        if( history[i].conf<KAL_HARD_TRESHOLD )
        {
            out << "%{u#f00}%{+u}";
        }
        else if( history[i].conf==1.00 )
        {
            out << "%{u#1d1}%{+u}";
        }
        else if( history[i].conf>KAL_CONF_TRESHOLD )
        {
            out << "%{u#16A1CF}%{+u}";
        }
        else
        {
            out << "%{u#CF8516}%{+u}";
        }
        out << history[i].word;

        out << "%{-u} ";
    }
    out << "\n";

    bar_file.close();
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

bool BtCaptain::isValidTime(double middle)
{
    if( middle<0.7 )
    {
        return false;
    }

    if( (middle>=BT_REC_SIZE-0.5) )
    {
        return false;
    }

    if( end>BT_REC_SIZE-0.2 )
    {
        return false;
    }

    return true;
}

void BtCaptain::addWord(QString word, double middle, double conf)
{
    BtWord word_buf;
    word_buf.word = word;
    word_buf.conf = conf;
    if( conf>KAL_HARD_TRESHOLD )
    {
        if( utterance.length() )
        {
            utterance += " ";
        }
        utterance += word;

        word_buf.time = middle-BT_DEC_TIMEOUT;
        words.append(word_buf);
    }

    word_buf.time = middle;
    history.append(word_buf);
}

int BtCaptain::lastWordIndex()
{
    if ( lastword.word.isEmpty() )
    {
        return 0;
    }

    int index = lastWordIndex(0.2);

    if( index==0 )
    {
        qDebug() << "Last word not found, change max_dist->0.5";
        index = lastWordIndex(0.5);
    }

    return index;
}

int BtCaptain::lastWordIndex(double max_dist)
{
    for( int i=0 ; i<conf->words.length() ; i++ )
    {
        if( conf->words[i].word==lastword.word )
        {
            double dist = qAbs(conf->words[i].time-lastword.time);
            if( dist<max_dist )
            {
                return i+1;
            }
        }
    }

    return 0;
}

void BtCaptain::shiftHistory()
{
    for( int i=0 ; i<history.length() ; i++ )
    {
        if( history[i].time<-4 )
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

// Return true if supplied word is the same as last word
int BtCaptain::isLastWord(QString word, double middle)
{
    double acc_margin = 0.2;
    if ( word==lastword.word )
    {
        if ( qAbs(middle-lastword.time)<acc_margin )
        {
            return true;
        }
    }

    return false;
}
