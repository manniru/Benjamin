#ifndef AB_STAT_H
#define AB_STAT_H

#include <QObject>
#include "config.h"
#include "backend.h"

#define AB_LIST_NAMES 1
#define AB_LIST_PATHS 2

#define AB_COLOR_LOW  0
#define AB_COLOR_HIGH 1
#define AB_COLOR_NORM 2


class AbStat: public QObject
{
    Q_OBJECT

public:
    explicit AbStat(QObject *ui, QObject *parent = nullptr);

    QString getStat(QString category = ""); // get total stat for wordlist
    void openCategory(QString category);
    QVector<int> countWords(QStringList file_list, int len);
    QString setFont(QString data, int val, int mean,
                    int var, int font_size = 24, int alignment = 0);
    int meanCount(QVector<int> count);
    int varCount(QVector<int> count, int mean);
    QFileInfoList listFiles(QString path);
    QStringList listFiles(QString path, int mode);
    QFileInfoList getAudioDirs();

public slots:

signals:

private slots:

private:
    void addWord(QString word, int count, int color);

    QObject* root;//root qml object
    QObject* editor;//word editor qml object
    QObject *status;//status qml object
};


#endif // AB_STAT_H
