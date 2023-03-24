#ifndef AB_STAT_H
#define AB_STAT_H

#include "config.h"
#include "backend.h"

#define AB_LIST_NAMES 1
#define AB_LIST_PATHS 2

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

#endif // AB_STAT_H
