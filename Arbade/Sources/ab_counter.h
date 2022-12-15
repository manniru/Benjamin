#ifndef ABCOUNTER_H
#define ABCOUNTER_H

#include <QDir>
#include "config.h"
#include "backend.h"

QString ab_getStat(QString category);
QVector<int> ab_countWords(QFileInfoList dir_list, int len);

#endif // ABCOUNTER_H
