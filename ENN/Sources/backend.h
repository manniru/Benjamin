#ifndef BACKEND_H
#define BACKEND_H

#include <QDebug>
#include <stdio.h>
#include <unistd.h>
#include <tiny_dnn/tiny_dnn.h>
#include "config.h"

int     getIntCommand(char *command);
QString getStrCommand(QString command);
QString getDiffTime(clock_t start);
QString getLDiffTime();
QStringList enn_listDirs(QString path);
QStringList enn_listImages(QString path);
QStringList enn_listDatas(QString path);
QStringList bt_parseLexicon(QString filename);
void enn_readENN(QString path, tiny_dnn::vec_t *out);
QString ab_getWslPath();
QString ab_getAudioPath();
#endif // BACKEND_H
