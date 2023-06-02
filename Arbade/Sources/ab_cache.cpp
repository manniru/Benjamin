#include "ab_cache.h"

AbCache::AbCache(QObject *parent) : QObject(parent)
{
    lexicon = bt_parseLexicon();
}

void AbCache::createCache()
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

QVector<QString>* AbCache::loadCacheFiles(QString category)
{
    QVector<QString> *files = NULL;
    int id = catToIndex(category);
    if( id<0 )
    {
        qDebug() << "Warning 321: cannot retrieve "
                    "category cache files (loadCacheFiles)";
        return files;
    }
    files = &cache_files[id];

    return files;
}

void AbCache::addCache(QString category, QString file)
{
    int cat_id = catToIndex(category);
    if( cat_id<0 )
    {
        qDebug() << "Error 321: cannot retrieve "
                    "category cache files (addCache)";
        return;
    }
    addCache(cat_id, file);
}

void AbCache::addCache(int cat_id, QString file)
{
    cache_files[cat_id].push_back(file);
    QVector<int> words_index = getWordsIndex(file);
    int len = words_index.length();
    for( int i=0 ; i<len ; i++ )
    {
        int word_id = words_index[i];
        cache_count[cat_id][word_id]++;
    }
}

void AbCache::deleteCache(QString category, QString path)
{
    int cat_id = catToIndex(category);
    if( cat_id<0 )
    {
        qDebug() << "Error 321: cannot retrieve "
                    "category cache files (deleteCache)";
        return;
    }
    int len = cache_files[cat_id].length();
    for( int i=0 ; i<len ; i++ )
    {
        if( cache_files[cat_id][i]==path )
        {
            deleteCache(cat_id, i);
            i--;
            len--;
        }
    }
}

void AbCache::deleteCache(QString category, int i)
{
    int cat_id = catToIndex(category);
    if( cat_id<0 )
    {
        qDebug() << "Error 321: cannot retrieve "
                    "category cache files (deleteCache)";
        return;
    }
    deleteCache(cat_id,i);
}

void AbCache::deleteCache(int cat_id, int i)
{
    QString removing_file = cache_files[cat_id][i];
    QVector<int> words_index = getWordsIndex(removing_file);
    int len = words_index.length();
    for( int j=0 ; j<len ; j++ )
    {
        int word_id = words_index[j];
        cache_count[cat_id][word_id]--;
    }
    cache_files[cat_id].remove(i);
}

void AbCache::addCategory(QString category)
{
    int cat_index = catToIndex(category);
    if( cat_index<0 )
    {
        qDebug() << "Error 321: cannot retrieve "
                    "category cache files (deleteCache)";
        return;
    }
    QFileInfoList dir_list = ab_getAudioDirs();
    int list_len = dir_list.length();

    for( int i=list_len-1 ; i>=cat_index ; i-- )
    {
        cache_files[i+1] = cache_files[i];
        cache_count[i+1] = cache_count[i];
    }
    QString cat_path = dir_list[cat_index].absoluteFilePath();
    cache_files[cat_index] = ab_listFiles(cat_path);
    cache_count[cat_index] = getCount(&cache_files[cat_index]);
}

int AbCache::catToIndex(QString category)
{
    int cat_id = -1;
    if( category==AB_UNVER_DIR )
    {
        cat_id = AB_UNVER_ID;
    }
    else if( category==AB_SHIT_DIR )
    {
        cat_id = AB_SHIT_ID;
    }
    else
    {
        QFileInfoList dir_list = ab_getAudioDirs();
        int len_dir = dir_list.size();
        for( int i=2 ; i<len_dir ; i++ )
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

QVector<int> AbCache::getCount(QVector<QString> *file_list)
{
    int lexicon_len = lexicon.length();
    QVector<int> words_index;
    QVector<int> count;
    count.resize(lexicon_len);
    count.fill(0);
    int file_num = file_list->size();

    for( int j=0 ; j<file_num ; j++ )
    {
        words_index = getWordsIndex(file_list->at(j));
        int words_num = words_index.length();
        for( int i=0 ; i<words_num ; i++ )
        {
            int index = words_index[i];
            if( index>=0 && index<lexicon_len )
            {
                count[index]++;
            }
        }
    }

    return count;
}

QVector<int> AbCache::getWordsIndex(QString filename)
{
    QFileInfo info(filename);
    filename = info.baseName();
    QVector<int> ret;
    QStringList words_index = filename.split("_",
                                     QString::SkipEmptyParts);
    int len = words_index.length();
    ret.resize(len);
    for( int i=0 ; i<len ; i++ )
    {
        ret[i] = words_index[i].toInt();
    }
    return ret;
}
