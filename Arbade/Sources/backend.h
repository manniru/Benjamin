#ifndef BACKEND_H
#define BACKEND_H

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

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

#define AB_LIST_NAMES  1
#define AB_LIST_PATHS  2
#define AB_UNVER_ID    0
#define AB_SHIT_ID     1
#define AB_UNVER_DIR   "unverified"
#define AB_SHIT_DIR    "shit"

int getIntCommand(char *command);
QString getStrCommand(QString command);
QString getDiffTime(clock_t start);
QString getLDiffTime();
QStringList bt_parseLexicon();
void bt_addLog(QString log);
void bt_mkDir(QString path);
QString ab_getWslPath();
QString ab_getAudioPath();
QVector<QString> ab_listFiles(QString path);
void ab_openCategory(QString category);
QFileInfoList ab_getAudioDirs();
QStringList ab_listFilesSorted(QString path);
QString getStatusStr(int status);
QString getVerifierStr(int verifier);
void ab_checkAuDir(QString dirname);
void ab_checkDir(QString path);
QString correctWinPath(QString path);

#endif // BACKEND_H
