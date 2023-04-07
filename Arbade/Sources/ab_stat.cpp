#include "ab_stat.h"
#include <time.h>
#include <stdlib.h>
#include <QQmlProperty>
#include <QVariant>

AbStat::AbStat(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    buttons = root->findChild<QObject *>("Buttons");
    status = root->findChild<QObject *>("Status");
    editor = root->findChild<QObject *>("WordList");
    rec_list = root->findChild<QObject *>("RecList");

    connect(root, SIGNAL(deleteSample(QString)),
            this, SLOT(deleteSample(QString)));

    parseLexicon();
    srand(time(NULL));
}

void AbStat::copyToOnline(QString filename)
{
    QFile file(filename);
    QFileInfo unver_file(file);

    QString online_dir = ab_getAudioPath();
    online_dir += "train\\online";
    QDir au_TrainDir(online_dir);

    if( !au_TrainDir.exists() )
    {
        qDebug() << "Creating" << online_dir
                 << " Directory";
#ifdef WIN32
        QString cmd = "mkdir " + online_dir;
        system(cmd.toStdString().c_str());
#else //OR __linux
        system("mkdir -p " KAL_AU_DIR "train/online");
#endif
    }
    QString new_path = online_dir + unver_file.fileName();
    file.copy(new_path);
    file.remove();
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

void AbStat::addRecList(QString word, QString path)
{
    QVariant word_v(word);
    QVariant path_v(path);

    QGenericArgument arg_word = Q_ARG(QVariant, word_v);
    QGenericArgument arg_path = Q_ARG(QVariant, path_v);

    QMetaObject::invokeMethod(rec_list, "addLine", arg_word,
                              arg_path);
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
//    addEmptyLine
    addWord("", -1, AB_COLOR_NORM);
}

void AbStat::createRecList(QString category)
{
    QString cat_path = ab_getAudioPath();
    cat_path += "train\\";
    cat_path += category;
    cat_path += "\\";
    qDebug() << "cat_path = " << cat_path;
    QDir cat_dir(cat_path);
    QFileInfoList samples_file = cat_dir.entryInfoList(QDir::Files,
                             QDir::Time | QDir::Reversed);
    qDebug() << "sm = " << samples_file.length();
    int samples_len = samples_file.length();

    if( samples_len>200 )
    {
        samples_len = 200;
    }
    for( int i=0 ; i<samples_len ; i++ )
    {
        QString file_name = samples_file[i].baseName();
        QStringList word_list = file_name.split('_');
        int world_list_len = word_list.length();
        QString word;
        for( int j=0 ; j<world_list_len ; j++)
        {
            int num = word_list[j].toInt();
            word += lexicon[num] + " ";
        }
        addRecList(word, samples_file[i].absolutePath());
    }
}

void AbStat::updateMeanVar(QVector<int> *count)
{
    int mean = meanCount(count);
    int var = varCount(count, mean);

    QQmlProperty::write(buttons, "mean", mean);
    QQmlProperty::write(buttons, "variance", var);
}

void AbStat::delWordSamples()
{
    QString dif = QQmlProperty::read(root, "ab_dif_words").toString();
    QStringList dif_words = dif.split("\n");
    int len = dif_words.length();
    QVector<int> del_list;

    for( int i=0 ; i<len ; i++ )
    {
        dif_words[i] = dif_words[i].split(".")[1].split("(")[0].trimmed();
        int result = wordToIndex(dif_words[i]);

        if( result>=0 && result<lexicon.size() )
        {
            del_list.push_back(result);
        }
    }

    len = del_list.size();

    if( len==0 )
    {
        return;
    }

    qDebug() << "delWordSamples" << del_list;

    QFileInfoList dir_list = ab_getAudioDirs();
    int len_dir = dir_list.size();

    if( len_dir==0 )
    {
        return;
    }

    for( int i=0 ; i<len_dir ; i++ )
    {
        QFileInfoList files_list = ab_listFiles(dir_list[i].
                                   absoluteFilePath());
        int len_files = files_list.size();

        for( int j=0 ; j<len_files ; j++ )
        {
            QStringList audio_words = files_list[j].baseName().split("_");

            for( int k=0 ; k<len ; k++ )
            {
                if( audio_words.contains(QString::number(del_list[k])) )
                {
                    QFile removing_file(files_list[j].absoluteFilePath());
                    qDebug() << "del" << files_list[j].absoluteFilePath();
                    removing_file.remove();
                }
            }
        }
    }
}

void AbStat::deleteSample(QString sample)
{
    QString category = QQmlProperty::read(editor, "category")
                                        .toString();
    QString file_path = ab_getAudioPath() + "train\\";
    file_path += category + "\\";

    QStringList sample_words = sample.split(" ",
                                     QString::SkipEmptyParts);
    int len = sample_words.length();
    for( int i=0 ; i<len ; i++ )
    {
        sample_words[i] = sample_words[i].remove(">").remove("<");
        file_path += QString::number(wordToIndex(sample_words[i]));
        file_path += "_";
    }
    file_path.chop(1); // removes last '_'
    file_path += ".wav";
    deleteFile(file_path);
}

void AbStat::deleteFile(QString path)
{
    QFile removing_file(path);
    qDebug() << "del" << path;
    if( removing_file.exists() )
    {
        removing_file.remove();
    }
}

int AbStat::wordToIndex(QString word)
{
    int len = lexicon.size();
    for( int i=0 ; i<len ; i++ )
    {
        if( lexicon[i]==word )
        {
            return i;
        }
    }
    return -1;
}

QString AbStat::idToWord(int id)
{
    if( id<lexicon.count() )
    {
        return lexicon[id];
    }
    else
    {
        return "<Unknown>";
    }
}

void AbStat::parseLexicon()
{
    QString wl_path = ab_getAudioPath() + "..\\word_list";
    lexicon = bt_parseLexicon(wl_path);
}
