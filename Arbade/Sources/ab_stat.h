#ifndef AB_STAT_H
#define AB_STAT_H

#include <QObject>
#include "config.h"
#include "backend.h"
#include "ab_phoneme.h"

#define AB_COLOR_LOW  0
#define AB_COLOR_HIGH 1
#define AB_COLOR_NORM 2

class AbStat: public QObject
{
    Q_OBJECT

public:
    explicit AbStat(QObject *ui, QObject *parent = nullptr);

    QVector<int> getCategoryCount(QString category);
    QVector<int> getAllCount();
    QVector<int> getCount(QStringList file_list);
    int meanCount(QVector<int> *count);
    int varCount(QVector<int> *count, int mean);
    void addWord(QString word, int count, QString phoneme);
    void addRecList(QString word, QString path);
    void createWordEditor(QString category);
    void createRecList(QString category);
    void updateMeanVar(QVector<int> *count);
    void delWordSamples();
    void copyToOnline(QString filename);
    QString idToWord(int id);

    QStringList lexicon;
    AbPhoneme *phoneme;

private slots:
    void deleteSample(QString sample);
    void create(QString catagorys);
    void update();

private:
    int wordToIndex(QString word);
    void deleteFile(QString path);

    QObject *root;//root qml object
    QObject *editor;//word editor qml object
    QObject *rec_list;//rec list qml object
    QObject *buttons;//buttons qml object
    QObject *status;//status qml object
};

#endif // AB_STAT_H
