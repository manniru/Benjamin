#ifndef BACKEND_H
#define BACKEND_H

#include <QtDBus>
#include <QDebug>
#include <stdio.h>
#include <unistd.h>

// KD_STATE_BLOWN: if detect words but
//                 cannot detect silence
// KD_STATE_NULL : if detect no words and
//                 recording become big

#define KD_STATE_SILENCE 1
#define KD_STATE_NORMAL  2
#define KD_STATE_BLOWN   3
#define KD_STATE_NULL    4

int getIntCommand(char *command);
QString getStrCommand(QString command);
QString getDiffTime(clock_t start);
QVector<QString> bt_parseLexicon(QString filename);

#endif // BACKEND_H
