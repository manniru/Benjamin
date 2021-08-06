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
    QFile conf_file(BT_CONF_PATH);
    QVector<QString> out_lines;

    if (!conf_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << BT_CONF_PATH;
        return;
    }

    sum_conf = 0;
    sum_det  = 0;
    utterance = "";

    while (!conf_file.atEnd())
    {
        QString line = QString(conf_file.readLine());
        line.remove('\n');
        QString p_line = processLine(line);
        out_lines.append(p_line);
    }

    if( out_lines.size()==0 )
    {
        avg_det = 0;
        avg_conf = 0;
    }
    else
    {
        printf("\n");
        avg_det  = sum_det/out_lines.size();
        avg_conf = sum_conf/out_lines.size();
    }

    conf_file.close();

    writeConfidence(out_lines);
}

QString BtConfidence::processLine(QString line)
{
    QStringList line_list = line.split(" ");
    QString out;

    if( line_list.size()==6 )
    {
        int index = line_list[4].toInt();
        double end = line_list[2].toDouble() + line_list[3].toDouble();
        line_list[4] = lexicon[index];

        isValidWord(line_list[4], line_list[2].toDouble(), end, line_list[5].toDouble());

        out = line_list.join(" ");
    }
    else
    {
        qDebug() << "Error Conf Size" << line << line_list.size();
    }

    return out;
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
//    if( avg_det<KAL_UDET_TRESHOLD && avg_conf<KAL_UCON_TRESHOLD )
//    {
//        if( avg_conf!=0 )
//        {
//            qDebug() << "Unvalid Utterance" << avg_det << avg_conf;
//        }
//        return false;
//    }
    if( utterance.isEmpty() )
    {
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

void BtConfidence::isValidWord(QString word, double start, double end, double conf)
{
    if( (start<BT_REC_SIZE-0.5-BT_DEC_TIMEOUT) ||
        (start>=BT_REC_SIZE-0.5) )
    {
        return;
    }

    if( end>BT_REC_SIZE-0.1 )
    {
        return;
    }

    if( conf>KAL_CONF_TRESHOLD )
    {
        sum_det++;
    }
    else if( word=="up" || word=="two" )
    {
        if( conf>KAL_HARD_TRESHOLD )
        {
            sum_det++;
        }
    }
    sum_conf += conf;

    if( conf>KAL_HARD_TRESHOLD )
    {
        if( !utterance.isEmpty() )
        {
            utterance += " ";
        }
        utterance += word;
    }

    printf("%s-%.2f(%.2f->%.2f) ", word.toStdString().c_str(), conf, start, end);
}
