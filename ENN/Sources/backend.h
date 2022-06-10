#ifndef BACKEND_H
#define BACKEND_H

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
#define KD_INFINITY_DB std::numeric_limits<double>::infinity()
#define KD_INFINITY_FL std::numeric_limits<float>::infinity()

int getIntCommand(char *command);
QString getStrCommand(QString command);
QString getDiffTime(clock_t start);
QString getLDiffTime();
QVector<QString> bt_parseLexicon(QString filename);
void bt_addLog(QString log);

#endif // BACKEND_H
