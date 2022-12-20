#ifndef AB_STAT_H
#define AB_STAT_H

#include "config.h"
#include "backend.h"

QString ab_getStat(QString category);
QVector<int> ab_countWords(QFileInfoList dir_list, int len);
QString setFont(QString data, int val, int mean,
                int var, int font_size=24, int alignment=0);
int ab_meanCount(QVector<int> count);
int ab_varCount(QVector<int> count, int mean);
QStringList ab_listDir(QString category);

#endif // AB_STAT_H
