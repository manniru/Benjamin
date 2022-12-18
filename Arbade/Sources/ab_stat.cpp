#include "ab_stat.h"
#include <QDebug>

QString ab_getStat(QString category)
{
    QString path = KAL_AU_DIR_WIN"train\\";
    path += category + "\\";
    QDir cat_dir(path);
    if( !cat_dir.exists() )
    {
        qDebug() << "Warning: Directory doesn't Exist,"
                 << "cannot generate statistics.";
        return "";
    }

    QStringList lexicon = bt_parseLexicon(BT_WORDLIST_PATH);
    int len = lexicon.length();

    QString result, data;

    QFileInfoList dir_list = cat_dir.entryInfoList(QDir::Files);
    QVector<int> count = ab_countWords(dir_list, len);

    int mean = ab_meanCount(count);
    int var = ab_varCount(count, mean);

    for( int i=0 ; i<len ; i++ )
    {
        data = QStringLiteral("%1").arg(i, 3, 10, QLatin1Char('0'));
        result += setFont(data, count[i], mean, var, 16);
        data = lexicon[i];
        result += setFont(data, count[i], mean, var) + "!";
        data = QString::number(count[i]);
        result += setFont(data, count[i], mean, var) + "!";
    }
    result += "mean!" + QString::number(mean) + "!";
    result += "var!" + QString::number(var) + "!";
    return result.trimmed();
}

QString setFont(QString data, int val, int mean, int var,
                int font_size, int alignment)
{
    // set font size
    QString result = "<font style=\"font-size: ";
    result += QString::number(font_size);
    result += "px;";

    if( alignment>0 )
    {
        result += "vertical-align: middle;";
    }

    // set font color
    if( val<mean-var )
    {
        result += "color: red;\"> ";
    }
    else if( val>mean+var )
    {
        result += "color: green;\"> ";
    }
    else
    {
        result += "color: black;\"> ";
    }
    result += data;
    result += "</font>";
    return result;
}

QVector<int> ab_countWords(QFileInfoList dir_list, int len)
{
    QString filename;
    QStringList words_index;
    QVector<int> count;
    count.resize(len);
    count.fill(0);
    int file_num = dir_list.size();

    for( int j=0 ; j<file_num ; j++ )
    {
        QFileInfo file=dir_list[j];
        filename = file.fileName();
        filename.remove(".wav");
        words_index = filename.split("_", QString::SkipEmptyParts);
        int words_num = words_index.length();
        for( int i=0 ; i<words_num ; i++ )
        {
            int index = words_index[i].toInt();
            if( index>=0 && index<len )
            {
                count[index]++;
            }
        }
    }
    return count;
}

int ab_meanCount(QVector<int> count)
{
    int len = count.size();
    int sum = 0;
    for( int i=0 ; i<len ; i++ )
    {
        sum += count[i];
    }
    return sum/len;
}

int ab_varCount(QVector<int> count, int mean)
{
    int len = count.size();
    int sum = 0;
    for( int i=0 ; i<len ; i++ )
    {
        sum += pow(count[i]-mean, 2);
    }
    return sqrt(sum/len);
}
