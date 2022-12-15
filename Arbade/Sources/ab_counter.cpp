#include "ab_counter.h"
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

    QString result;

    QFileInfoList dir_list = cat_dir.entryInfoList(QDir::Files);
    QVector<int> count = ab_countWords(dir_list, len);

    for( int i=0 ; i<len ; i++ )
    {
        QString id_string = "<font style=\"font-size:16px\";";
        id_string += "margin-bottom: 50px;> ";
        id_string += QStringLiteral("%1").arg(i, 3, 10, QLatin1Char('0'));
        id_string += " </font>";
        result += id_string;
        result += lexicon[i] + "!" + QString::number(count[i]) + "!";
    }
    return result.trimmed();
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
