#ifndef AB_STAT_H
#define AB_STAT_H

#include <QObject>
#include "config.h"
#include "backend.h"
#include "ab_telegram.h"
#include "ab_phoneme.h"
#include "ab_cache.h"

#define AB_RECORD_MODE  0
#define AB_VERIFY_MODE  1
#define AB_EFALSE_MODE    2

class AbStat: public QObject
{
    Q_OBJECT

public:
    explicit AbStat(QObject *ui, AbCache *ch, AbTelegram *tel,
                    QObject *parent = nullptr);

    QVector<int>* getCategoryCount(QString category);
    QVector<int>  getAllCount();
    int  wordToIndex(QString word);
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

    AbCache *cache;
    AbPhoneme *phoneme;

signals:

private slots:
    void deleteSample(QString sample);
    void create(QString catagorys);
    void update();

private:
    int meanCount(QVector<int> *count);
    int varCount(QVector<int> *count, int mean);
    int haveWord(int word_id, QString path);

    QObject *root;//root qml object
    QObject *editor;//word editor qml object
    QObject *rec_list;//rec list qml object
    QObject *buttons;//buttons qml object
    QObject *status;//status qml object
    AbTelegram *telegram;
};

#endif // AB_STAT_H
