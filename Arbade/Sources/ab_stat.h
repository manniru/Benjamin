#ifndef AB_STAT_H
#define AB_STAT_H

#include <QObject>
#include "config.h"
#include "backend.h"

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
    QString setFont(QString data, int val, int mean,
                    int var, int font_size = 24, int alignment = 0);
    int meanCount(QVector<int> *count);
    int varCount(QVector<int> *count, int mean);
    void addWord(QString word, int count, int color);
    void createWordEditor(QString category);
    void updateMeanVar(QVector<int> *count);

private:
    QObject *root;//root qml object
    QObject *editor;//word editor qml object
    QObject *buttons;//buttons qml object
    QObject *status;//status qml object

    QStringList lexicon;
};

#endif // AB_STAT_H
