#ifndef MM_WATCHER_H
#define MM_WATCHER_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>

#include "mm_config.h"

class MmWatcher : public QObject
{
    Q_OBJECT
public:
    MmWatcher(QObject *parent = nullptr);


private slots:
    void timeout();

private:
    bool isAppRunning(QString name);
    void runApp(QString appName);
    HANDLE run(QString path);
    int appIsOpen(QString appName);
    long findProcessId(QString processname);

    QTimer *timer;
    QStringList appNames;
    QVector<HANDLE> app_h;
};

#endif // MM_WATCHER_H
