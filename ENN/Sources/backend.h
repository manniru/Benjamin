#ifndef BACKEND_H
#define BACKEND_H

#include <QDebug>
#include <stdio.h>
#include <unistd.h>

int     getIntCommand(char *command);
QString getStrCommand(QString command);
QString getDiffTime(clock_t start);
QString getLDiffTime();
QStringList enn_listDirs(QString path);
QStringList enn_listImages(QString path);

#endif // BACKEND_H
