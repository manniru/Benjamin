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
}

QVector<int> AbStat::getCount(QVector<QString> *file_list)
{
    int lexicon_len = lexicon.length();
    QString filename;
    QStringList words_index;
    QVector<int> count;
    count.resize(lexicon_len);
    count.fill(0);
    int file_num = file_list->size();

    for( int j=0 ; j<file_num ; j++ )
    {
        QFileInfo info(file_list->at(j));
        filename = info.baseName();

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

void AbStat::addWord(QString word, int count, QString phoneme)
{
    QVariant word_v(word);
    QVariant phoneme_v(phoneme);

    QGenericArgument arg_word    = Q_ARG(QVariant, word_v);
    QGenericArgument arg_count   = Q_ARG(QVariant, count);
    QGenericArgument arg_phoneme = Q_ARG(QVariant, phoneme_v);

    QMetaObject::invokeMethod(editor, "addWord", arg_word,
                              arg_count, arg_phoneme);
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

QVector<int>* AbStat::getCategoryCount(QString category)
{
    QVector<int> *ret = NULL;
    int id = catToIndex(category);
    ret = &cache_count[id];
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
        for( int j=0 ; j<lex_len ; j++ )
        {
            ret[j] += cache_count[i][j];
        }
    }

    return ret;
}

void AbStat::createWordEditor(QString category)
{
    QVector<int> *count = getCategoryCount(category);
    updateMeanVar(count);

    int lexicon_len = lexicon.length();
    for( int i=0 ; i<lexicon_len ; i++ )
    {
        QString phon = phoneme->getPhoneme(lexicon[i]);
        if( count!=NULL )
        {
            addWord(lexicon[i], count->at(i), phon);
        }
        else
        {
            addWord(lexicon[i], 0, phon);
        }
    }
//    addEmptyLine
    addWord("", -1, "");
}

void AbStat::createRecList(QString category)
{
    QVector<QString> *files = loadCacheFiles(category);
    if( files==NULL )
    {
        qDebug() << "Info 85: empty category";
        return;
        return;
    }

    int samples_len = files->length();
    int lexicon_len = lexicon.length();
    int start_i = 0;
    if( samples_len>AB_MAX_RECLIST )
    {
        start_i = samples_len-AB_MAX_RECLIST;
    }
    for( int i=start_i ; i<samples_len ; i++ )
    {
        QFileInfo info(files->at(i));
        QString file_name = info.fileName();
        file_name = file_name.remove(".wav");
        QStringList name_extended = file_name.split(".");
        file_name = info.baseName();
        QStringList word_list = file_name.split('_');
        int world_list_len = word_list.length();
        QString word;
        for( int j=0 ; j<world_list_len ; j++)
        {
            int num = word_list[j].toInt();

            if( num<lexicon_len )
            {
                word += lexicon[num] + " ";
            }
            else
            {
                qDebug() << "Error 161: Out of range sample"
                         << info.path();
                return;
            }
        }
        if( name_extended.size()>1 )
        {
            word += "(" + name_extended[1] + ")";
        }
        addRecList(word, info.absoluteFilePath());
    }
}

void AbStat::updateMeanVar(QVector<int> *count)
{
    int mean = meanCount(count);
    int var = varCount(count, mean);

    QQmlProperty::write(buttons, "mean", mean);
    QQmlProperty::write(buttons, "variance", var);
}

void AbStat::delAllSamples(int word_id)
{
    QFileInfoList dir_list = ab_getAudioDirs();
    int len_dir = dir_list.size();
    if( len_dir==0 )
    {
        return;
    }

    for( int i=0 ; i<len_dir ; i++ )
    {
        int len_files = cache_files[i].size();

        for( int j=0 ; j<len_files ; j++ )
        {
            QString path = cache_files[i][j];

            if( haveWord(word_id, path) )
            {
                QFile file(path);
                qDebug() << "del" << path;
                file.remove();
                deleteCache(i, j);
                len_files--;
                j--;
            }
        }
    }
}

// return true if path contains the word id, wether is wrong (negative)
// or normal word_id
int AbStat::haveWord(int word_id, QString path)
{
    QFileInfo info(path);
    //baseName remove all after the first .
    QStringList words = info.baseName().split("_");
    int words_len = words.length();

    for( int i=0 ; i<words_len ; i++ )
    {
        int w_id = words[i].toInt();
        w_id = qAbs(w_id);
        if( w_id==word_id )
        {
            return 1;
        }
    }

    return 0;
}

// delete ENN Sample based on word_id
void AbStat::deleteEnn(int word_id)
{
    // remove wrong sample
    QString wrong_dir = ab_getAudioPath() + "wrong";
    QVector<QString> wrong_files = ab_listFiles(wrong_dir);
    int wrong_len = wrong_files.size();
    for( int j=0 ; j<wrong_len ; j++ )
    {
        QString path = wrong_files[j];

        if( haveWord(word_id, path) )
        {
            QFile file(path);
            qDebug() << "del" << path;
            file.remove();
        }
    }

    if( word_id>=lexicon.length() )
    {
        qDebug() << "Error 149: word_id exceeds lexicon len";
        return;
    }
    QString word = lexicon[word_id];
    // remove wrong sample
    QString enn_path = ab_getAudioPath() + "enn";
    enn_path += QDir::separator() + word;
    qDebug() << "enn_path"
             << enn_path;
    QDir enn_dir(enn_path);
    enn_dir.removeRecursively();
}

void AbStat::create(QString category)
{
    phoneme = new AbPhoneme();
    lexicon = bt_parseLexicon();
    srand(time(NULL));

    createCache();
    createWordEditor(category);
    createRecList(category);
}

void AbStat::update()
{
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
    QFile removing_file(file_path);
    qDebug() << "del" << file_path;
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

int AbStat::catToIndex(QString category)
{
    int cat_id = AB_UNVER_ID;
    if( category!=AB_UNVER )
    {
        QFileInfoList dir_list = ab_getAudioDirs();
        int len_dir = dir_list.size();
        for( int i=1 ; i<len_dir ; i++ )
        {
            QString dir_name = dir_list[i].baseName();
            if( dir_name==category )
            {
                return i;
            }
        }
    }
    return cat_id;
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

void AbStat::moveToOnline(int id)
{
    QFileInfoList dir_list = ab_getAudioDirs();
    int len_dir = dir_list.size();

    for( int i=1 ; i<len_dir ; i++ )
    {
        QString dir_name = dir_list[i].baseName();
        if( dir_name=="online" )
        {
            cache_files[i] << cache_files[0][id];
            deleteCache(AB_UNVER_ID, id);
        }
    }
}

////////// Cache /////////////////
void AbStat::createCache()
{
    QFileInfoList dir_list = ab_getAudioDirs();
    int len_dir = dir_list.size();

    for( int i=0 ; i<len_dir ; i++ )
    {
        QString cat_path = dir_list[i].absoluteFilePath();
        cache_files[i] = ab_listFiles(cat_path);
        cache_count[i] = getCount(&cache_files[i]);
    }

    emit cacheCreated();
}

QVector<QString>* AbStat::loadCacheFiles(QString category)
{
    QVector<QString> *files = NULL;
    int id = catToIndex(category);
    files = &cache_files[id];

    return files;
}

void AbStat::deleteCache(QString category, int i)
{
    int cat_id = catToIndex(category);
    deleteCache(cat_id,i);
}

void AbStat::deleteCache(int cat_id, int i)
{
    cache_files[cat_id].remove(i);
    // cache_count[i] = getCount(&cache_files[i]);
}
