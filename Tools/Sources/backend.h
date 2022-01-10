#ifndef BACKEND_H
#define BACKEND_H

#include <QtDBus>
#include <QDebug>
#include <stdio.h>
#include <unistd.h>

#define KD_STATE_SILENCE 1
#define KD_STATE_NORMAL  2

int getIntCommand(char *command);
QString getStrCommand(QString command);
QString getDiffTime(clock_t start);

#endif // BACKEND_H
