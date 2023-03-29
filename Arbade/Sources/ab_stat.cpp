#include "ab_stat.h"
#include <QQmlProperty>

int cat_mean = 0;
int cat_var  = 0;

AbStat::AbStat(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    status = root->findChild<QObject *>("Status");
    editor = root->findChild<QObject*>("WordList");
}

QString AbStat::getStat(QString category)
{
    QFileInfoList dir_list;

    if( category.length() )
    {
        QString path = ab_getAudioPath();
        path += "train\\" + category + "\\";
        QFileInfo category_dir(path);
        dir_list.append(category_dir);
    }
    else
    {
        dir_list = getAudioDirs();
    }

    int len_dir = dir_list.size();

    if( len_dir==0 )
    {
        return "";
    }

    QString wl_path = ab_getAudioPath() + "..\\word_list";
    QStringList lexicon = bt_parseLexicon(wl_path);
    int len = lexicon.length();
    QVector<int> tot_count;
    tot_count.resize(len);
    tot_count.fill(0);

    for( int i=0 ; i<len_dir ; i++ )
    {
        QStringList files_list = listFiles(dir_list[i].absoluteFilePath(),
                                           AB_LIST_NAMES);
        //        qDebug() << ">>dir:" << dir_list[i].absoluteFilePath()
        //                 << "\n" << files_list;
        QVector<int> count = countWords(files_list, len);

        for( int j=0 ; j<len ; j++ )
        {
            tot_count[j] += count[j];
        }
    }

    cat_mean = meanCount(tot_count);
    cat_var = varCount(tot_count, cat_mean);

    QQmlProperty::write(status, "mean", cat_mean);
    QQmlProperty::write(status, "variance", cat_var);

    QString result;

    for( int i=0 ; i<len ; i++ )
    {
        result += "(" + QString::number(tot_count[i]) + ")\n";
    }

    return result.trimmed();
}

QFileInfoList AbStat::getAudioDirs()
{
    QString path = ab_getAudioPath() + "train\\";
    QDir dir(path);

    if( !dir.exists() )
    {
        qDebug() << "Warning: train directory doesn't Exist,"
                 << "cannot generate statistics.";
        return QFileInfoList();
    }

    QFileInfoList dir_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    path = ab_getAudioPath() + "unverified\\";
    QFileInfo unver(path);

    if( !unver.exists() )
    {
        qDebug() << "Warning: unverified directory doesn't Exist,"
                 << "cannot generate statistics.";
        return QFileInfoList();
    }

    dir_list.append(unver); // add unverified dir to list of train category dirs

    return dir_list;
}

void AbStat::openCategory(QString category)
{
    QString path = ab_getAudioPath();

    if( category!="unverified" )
    {
        path += "train\\";
    }

    path += category + "\\";

    QString cmd = "explorer.exe " + path;
    system(cmd.toStdString().c_str());
}

QFileInfoList AbStat::listFiles(QString path)
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

QStringList AbStat::listFiles(QString path, int mode)
{
    QStringList ret;
    QFileInfoList dir_list = listFiles(path);
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

QString AbStat::setFont(QString data, int val, int mean, int var,
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
    if( val<(mean-var) )
    {
        result += "color: #c4635e;\"> ";
    }
    else if( val>(mean+var) )
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

QVector<int> AbStat::countWords(QStringList file_list, int len)
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

int AbStat::meanCount(QVector<int> count)
{
    int len = count.size();
    int sum = 0;

    for( int i=0 ; i<len ; i++ )
    {
        sum += count[i];
    }

    if( len )
    {
        return sum / len;
    }

    return 0;
}

int AbStat::varCount(QVector<int> count, int mean)
{
    int len = count.size();
    int sum = 0;

    for( int i=0 ; i<len ; i++ )
    {
        sum += pow(count[i] - mean, 2);
    }

    if( len )
    {
        return sqrt(sum / len);
    }

    return 0;
}

void AbStat::addWord(QString word, int count, int color)
{
    QVariant word_v(word);
    QVariant color_v(word);
    if( color==AB_COLOR_LOW )
    {
        color_v = QVariant("#cb6565"); //red
    }
    else if( color==AB_COLOR_HIGH )
    {
        color_v = QVariant("#80bf73"); //green
    }
    else // neutral
    {
        color_v = QVariant("#80bf73"); //gray
    }

    QGenericArgument arg_word  = Q_ARG(QVariant, word_v);
    QGenericArgument arg_count = Q_ARG(QVariant, count);
    QGenericArgument arg_color = Q_ARG(QVariant, color_v);

    QMetaObject::invokeMethod(editor, "addWord", arg_word,
                              arg_count, arg_color);
}
