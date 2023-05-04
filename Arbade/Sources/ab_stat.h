#ifndef AB_STAT_H
#define AB_STAT_H

#include <QObject>
#include "config.h"
#include "backend.h"
#include "ab_phoneme.h"

#define AB_MAX_CAT     100
#define AB_MAX_RECLIST 200
#define AB_UNVER_ID    0
#define AB_UNVER       "unverified"

class AbStat: public QObject
{
    Q_OBJECT

public:
    explicit AbStat(QObject *ui, QObject *parent = nullptr);

    QVector<int>* getCategoryCount(QString category);
    QVector<int>  getAllCount();
    int  wordToIndex(QString word);
    int  catToIndex(QString category);
    void addWord(QString word, int count, QString phoneme);
    void addRecList(QString word, QString path);
    void createWordEditor(QString category);
    void createRecList(QString category);
    void updateMeanVar(QVector<int> *count);
    void moveToOnline(int id);
    QString idToWord(int id);
    void deleteCache(QString category, int i);
    void deleteCache(int cat_id, int i);
    void delAllSamples(int word_id);
    void deleteEnn(int word_id);

    QString dif_editor;
    QStringList lexicon;
    QVector<QString> cache_files[AB_MAX_CAT];
    QVector<int> cache_count[AB_MAX_CAT];
    AbPhoneme *phoneme;

signals:
    void cacheCreated();

private slots:
    void deleteSample(QString sample);
    void create(QString catagorys);
    void update();

private:
    void createCache();
    QVector<QString>* loadCacheFiles(QString category);
    QVector<int> getCount(QVector<QString> *file_list);
    int meanCount(QVector<int> *count);
    int varCount(QVector<int> *count, int mean);
    int haveWord(int word_id, QString path);

    QObject *root;//root qml object
    QObject *editor;//word editor qml object
    QObject *rec_list;//rec list qml object
    QObject *buttons;//buttons qml object
    QObject *status;//status qml object
};

#endif // AB_STAT_H
