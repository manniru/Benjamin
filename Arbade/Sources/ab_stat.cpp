#include "ab_stat.h"
int cat_mean = 0;
int cat_var  = 0;

QString ab_getStat(QString category)
{
    QFileInfoList dir_list;
    if( category.length() )
    {
        QString path = KAL_AU_DIR_WIN"train\\"+category+"\\";
        QFileInfo category_dir(path);
        dir_list.append(category_dir);
    }
    else
    {
        dir_list = ab_getAudioDirs();
    }
    int len_dir = dir_list.size();
    if( len_dir==0 )
    {
        return "";
    }

    QStringList lexicon = bt_parseLexicon(BT_WORDLIST_PATH);
    int len = lexicon.length();
    QVector<int> tot_count;
    tot_count.resize(len);
    tot_count.fill(0);

    for( int i=0 ; i<len_dir ; i++ )
    {
        QStringList files_list = ab_listFiles(dir_list[i].absoluteFilePath(),
                                              AB_LIST_NAMES);
        QVector<int> count = ab_countWords(files_list, len);
        for( int j=0 ; j<len ; j++ )
        {
            tot_count[j] += count[j];
        }
    }
    cat_mean = ab_meanCount(tot_count);
    cat_var = ab_varCount(tot_count, cat_mean);

    QString result;
    for( int i=0 ; i<len ; i++ )
    {
        result += "(" + QString::number(tot_count[i]) + ")\n";
    }
    return result.trimmed();
}

QFileInfoList ab_getAudioDirs()
{
    QString path = KAL_AU_DIR_WIN"train\\";
    QDir dir(path);
    if( !dir.exists() )
    {
        qDebug() << "Warning: Audio Directory doesn't Exist,"
                 << "cannot generate statistics.";
        return QFileInfoList();
    }
    QFileInfoList dir_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    path = KAL_AU_DIR_WIN"unverified\\";
    QFileInfo unver(path);
    if( !unver.exists() )
    {
        qDebug() << "Warning: unverified Directory doesn't Exist,"
                 << "cannot generate statistics.";
        return QFileInfoList();
    }
    dir_list.append(unver); // add unverified dir to list of train category dirs

    return dir_list;
}

//QString ab_getStat(QString category)
//{
//    QString path;
//    if( category=="unverified" )
//    {
//        path = KAL_AU_DIR_WIN + category + "\\";
//    }
//    else
//    {
//        path = KAL_AU_DIR_WIN"train\\";
//        path += category + "\\";
//    }
//    QStringList files_list = ab_listFiles(path, AB_LIST_NAMES);
//    if( files_list.length()<=0 )
//    {
//        return "";
//    }
//    QStringList lexicon = bt_parseLexicon(BT_WORDLIST_PATH);
//    int len = lexicon.length();
//    QVector<int> count = ab_countWords(files_list, len);

//    cat_mean = ab_meanCount(count);
//    cat_var = ab_varCount(count, cat_mean);
//    QString result, data;

//    for( int i=0 ; i<len ; i++ )
//    {
//        data = QStringLiteral("%1").arg(i, 3, 10, QLatin1Char('0'));
//        result += setFont(data, count[i], cat_mean, cat_var);
//        data = lexicon[i];
//        result += setFont(data, count[i], cat_mean, cat_var) + " ";
//        data = QString::number(count[i]);
//        result += setFont(data, count[i], cat_mean, cat_var) + "!";
//    }
//    return result.trimmed();
//}

QString ab_getMeanVar()
{
    QString result = QString::number(cat_mean) + "!";
    result += QString::number(cat_var) + "!";
    return result;
}

QFileInfoList ab_listFiles(QString path)
{
    QDir cat_dir(path);
    if( !cat_dir.exists() )
    {
        qDebug() << "Warning: Directory doesn't Exist,"
                 << "cannot generate statistics.";
        return QFileInfoList();
    }
    QFileInfoList dir_list = cat_dir.entryInfoList(QDir::Files,
                                 QDir::Time | QDir::Reversed);
    return dir_list;
}

QStringList ab_listFiles(QString path, int mode)
{
    QStringList ret;
    QFileInfoList dir_list = ab_listFiles(path);
    int len = dir_list.size();
    for( int i=0 ; i<len ; i++ )
    {
        if( mode==AB_LIST_PATHS )
        {
            ret.push_back(dir_list[i].absoluteFilePath());
        }
        else
        {
            ret.push_back(dir_list[i].fileName());
        }
    }
    return ret;
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
        result += "color: #c4635e;\"> ";
    }
    else if( val>mean+var )
    {
        result += "color: #36b245;\"> ";
    }
    else
    {
        result += "color: #b3b3b3;\"> ";
    }
    result += data;
    result += "</font>";
    return result;
}

QVector<int> ab_countWords(QStringList file_list, int len)
{
    QString filename;
    QStringList words_index;
    QVector<int> count;
    count.resize(len);
    count.fill(0);
    int file_num = file_list.size();

    for( int j=0 ; j<file_num ; j++ )
    {
        filename = file_list[j];
        filename.remove(".wav");
        int dot_index = filename.indexOf(".");
        if( dot_index>=0 )
        {
            filename.truncate(dot_index);
        }
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
