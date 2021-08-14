#include "bt_confidence.h"
#include <QDebug>

BtConfidence::BtConfidence(QObject *parent) : QObject(parent)
{
    parseWords(BT_WORDS_PATH);
    setbuf(stdout,NULL);

}

void BtConfidence::parseWords(QString filename)
{
    QFile words_file(filename);

    if (!words_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << filename;
        return;
    }

    lexicon.clear();

    while (!words_file.atEnd())
    {
        QString line = QString(words_file.readLine());
        QStringList line_list = line.split(" ");
        lexicon.append(line_list[0]);
    }

    words_file.close();
}

void BtConfidence::parseConfidence()
{
    if( words.length() )
    {
        lastword = words.last();
    }
    else
    {
        lastword.time = BT_REC_SIZE-BT_DEC_TIMEOUT-1;
        lastword.word = "";
    }
    words.clear();
    shiftHistory();

    QFile conf_file(BT_CONF_PATH);
    QVector<QString> out_lines;

    if (!conf_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << BT_CONF_PATH;
        return;
    }

    utterance = "";

    while (!conf_file.atEnd())
    {
        QString line = QString(conf_file.readLine());
        line.remove('\n');
        QString p_line = processLine(line);
        out_lines.append(p_line);
    }

    if( out_lines.size() )
    {
        printf("\n");
    }

    conf_file.close();

    writeConfidence(out_lines);
    writeBarResult();
}

QString BtConfidence::processLine(QString line)
{
    QStringList line_list = line.split(" ");
    QString out;

    if( line_list.size()==6 )
    {
        int index = line_list[4].toInt();
        double start = line_list[2].toDouble();
        double end   = start + line_list[3].toDouble();
        line_list[4] = lexicon[index];

        if( isValidTime(line_list[4], start, end) )
        {
            double middle = (start+end)/2;
            double conf   = line_list[5].toDouble();
            addWord(line_list[4], middle, conf);
            printf("%s-%.2f(%.2f->%.2f) ", line_list[4].toStdString().c_str()
                   , conf, start, end);
        }

        out = line_list.join(" ");
    }
    else
    {
        qDebug() << "Error Conf Size" << line << line_list.size();
    }

    return out;
}

void BtConfidence::writeBarResult()
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

void BtConfidence::writeConfidence(QVector<QString> lines)
{
    QFile conf_file(BT_CONF_PATH);

    if (!conf_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << BT_CONF_PATH;
        return;
    }

    QTextStream out(&conf_file);

    for( int i=0 ; i<lines.size() ; i++ )
    {
        out << lines[i] << "\n";
    }

    conf_file.close();
}

bool BtConfidence::isValidUtterance()
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

void BtConfidence::printWords(QString words)
{
    QStringList word_list = words.split(" ");

    for( int i=1 ; i<word_list.size() ; i++ )
    {
        printf("%-10s ", word_list[i].toStdString().c_str());
    }
    printf("\n");
}

QString BtConfidence::getUtterance()
{
    return utterance;
}

bool BtConfidence::isValidTime(QString word, double start, double end)
{
    double middle = (start+end)/2;

    if( middle<lastword.time )
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

    if( isLastWord(word, middle) )
    {
//        qDebug() << "last word are the same" << word << lastword.time << middle;
        return false;
    }

    return true;
}

void BtConfidence::addWord(QString word, double middle, double conf)
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

void BtConfidence::shiftHistory()
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

double BtConfidence::getAvgConfidence()
{
    double sum = 0;
    for( int i=0 ; i<history.length() ; i++ )
    {
        sum += history[i].conf;
    }

    return sum/history.length();
}

double BtConfidence::getAvgDetection()
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
int BtConfidence::isLastWord(QString word, double middle)
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
