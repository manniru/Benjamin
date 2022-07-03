#ifndef BACKEND_H
#define BACKEND_H

#include <QDebug>
#include <stdio.h>
#include <unistd.h>
#include <tiny_dnn/tiny_dnn.h>

int     getIntCommand(char *command);
QString getStrCommand(QString command);
QString getDiffTime(clock_t start);
QString getLDiffTime();
QStringList enn_listDirs(QString path);
QStringList enn_listImages(QString path);
QStringList enn_listDatas(QString path);
void enn_readENN(QString path, tiny_dnn::vec_t *out);

#endif // BACKEND_H
