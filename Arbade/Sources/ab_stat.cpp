#include "ab_stat.h"
#include <QQmlProperty>

AbStat::AbStat(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    buttons = root->findChild<QObject *>("Buttons");
    status = root->findChild<QObject *>("Status");
    editor = root->findChild<QObject*>("WordList");


    QString wl_path = ab_getAudioPath() + "..\\word_list";
    lexicon = bt_parseLexicon(wl_path);
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

QVector<int> AbStat::getCount(QStringList file_list)
{
    int lexicon_len = lexicon.length();
    QString filename;
    QStringList words_index;
    QVector<int> count;
    count.resize(lexicon_len);
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
            if( index>=0 && index<lexicon_len )
            {
                count[index]++;
            }
        }
    }

    return count;
}

int AbStat::meanCount(QVector<int> *count)
{
    int len = count->size();
    int sum = 0;

    for( int i=0 ; i<len ; i++ )
    {
        sum += count->at(i);
    }

    if( len )
    {
        return sum / len;
    }

    return 0;
}

int AbStat::varCount(QVector<int> *count, int mean)
{
    int len = count->size();
    int sum = 0;

    for( int i=0 ; i<len ; i++ )
    {
        sum += pow(count->at(i) - mean, 2);
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

    QMetaObject::invokeMethod(editor, "addWord", arg_word,
                              arg_count);
}

QVector<int> AbStat::getCategoryCount(QString category)
{
    QString cat_path = ab_getAudioPath();
    cat_path += "train\\" + category + "\\";

    QStringList samples = ab_listFiles(cat_path, AB_LIST_NAMES);
    QVector<int> ret = getCount(samples);

    return ret;
}

QVector<int> AbStat::getAllCount()
{
    QVector<int> ret;
    QFileInfoList dir_list = ab_getAudioDirs();
    int len_dir = dir_list.size();
    if( len_dir==0 )
    {
        return ret;
    }

    int lex_len = lexicon.length();
    ret.resize(lex_len);
    ret.fill(0);
    for( int i=0 ; i<len_dir ; i++ )
    {
        QStringList files_list = ab_listFiles(dir_list[i].absoluteFilePath(),
                                           AB_LIST_NAMES);
        QVector<int> count = getCount(files_list);

        for( int j=0 ; j<lex_len ; j++ )
        {
            ret[j] += count[j];
        }
    }

    return ret;
}

void AbStat::createWordEditor(QString category)
{
    QVector<int> count = getCategoryCount(category);
    updateMeanVar(&count);

    int lexicon_len = lexicon.length();
    for( int i=0 ; i<lexicon_len ; i++ )
    {
        addWord(lexicon[i], count[i], AB_COLOR_NORM);
    }
}

void AbStat::updateMeanVar(QVector<int> *count)
{
    int mean = meanCount(count);
    int var = varCount(count, mean);

    QQmlProperty::write(buttons, "mean", mean);
    QQmlProperty::write(buttons, "variance", var);
}
