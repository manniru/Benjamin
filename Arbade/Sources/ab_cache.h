#ifndef AB_CACHE_H
#define AB_CACHE_H

#include <QObject>
#include <QString>

#include "backend.h"

#define AB_MAX_CAT     100
#define AB_MAX_RECLIST 200
#define AB_UNVER_ID    0
#define AB_SHIT_ID     1

class AbCache: public QObject
{
    Q_OBJECT

public:
    explicit AbCache(QObject *parent = nullptr);

    void createCache();
    QVector<QString>* loadCacheFiles(QString category);
    void deleteCache(QString category, int i);
    void deleteCache(int cat_id, int i);
    QVector<int> getCount(QVector<QString> *file_list);

    QVector<QString> cache_files[AB_MAX_CAT];
    QVector<int> cache_count[AB_MAX_CAT];
    int  catToIndex(QString category);

signals:
    void cacheCreated();

private:
    QStringList lexicon;
};

#endif // AB_CACHE_H
