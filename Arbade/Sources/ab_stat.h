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

QString ab_getStat(QString category=""); // get total stat for wordlist
QString ab_getMeanVar();
QString ab_getAudioPath();
void ab_openCategory(QString category);
QVector<int> ab_countWords(QStringList file_list, int len);
QString setFont(QString data, int val, int mean,
                int var, int font_size=24, int alignment=0);
int ab_meanCount(QVector<int> count);
int ab_varCount(QVector<int> count, int mean);
QFileInfoList ab_listFiles(QString path);
QStringList ab_listFiles(QString path, int mode);
QFileInfoList ab_getAudioDirs();

class AbStat: public QObject
{
    Q_OBJECT
public:
    explicit AbStat(QObject *ui, QObject *parent = nullptr);

private slots:

private:
    void addWord(QString word, int count, int color);

    QObject* root;//root qml object
    QObject* editor;//word editor qml object
};


#endif // AB_STAT_H
