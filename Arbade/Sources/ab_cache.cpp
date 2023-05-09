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
    files = &cache_files[id];

    return files;
}

void AbCache::deleteCache(QString category, int i)
{
    int cat_id = catToIndex(category);
    deleteCache(cat_id,i);
}

void AbCache::deleteCache(int cat_id, int i)
{
    cache_files[cat_id].remove(i);
    // cache_count[i] = getCount(&cache_files[i]);
}

int AbCache::catToIndex(QString category)
{
    int cat_id = AB_UNVER_ID;
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
