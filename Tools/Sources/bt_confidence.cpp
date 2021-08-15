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
    words.clear();

    if (!conf_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << BT_CONF_PATH;
        return;
    }

    while (!conf_file.atEnd())
    {
        QString line = QString(conf_file.readLine());
        line.remove('\n');
        QString p_line = processLine(line);
        out_lines.append(p_line);
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
        double start = line_list[2].toDouble();
        double end   = start + line_list[3].toDouble();
        line_list[4] = lexicon[index];

        BtWord buf;
        buf.time = (start+end)/2;
        buf.conf = line_list[5].toDouble();
        buf.word = line_list[4];

        words.append(buf);

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
