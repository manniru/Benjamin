#ifndef MM_KEY_EXECUTER_H
#define MM_KEY_EXECUTER_H

#include <stdio.h>
#include <Windows.h>
#include <QVector>
#include "mm_app_launcher.h"
#include "mm_virt.h"
#include "mm_lua.h"

class MmKeyExec : public QObject
{
    Q_OBJECT
public:
    explicit MmKeyExec(MmVirt *vi,
                       QObject *parent = nullptr);
    ~MmKeyExec();

    int supress_r = 0; //suprress release flag for win key
    int win_p = 0;
    int execWinKey(int key_code);
    void goToSleep(int *emul_mode);

public slots:
    void delayedExec();

private:
    int execWinNum(int key_code);

    QTimer *timer;
    MmLua *lua;
    MmVirt *virt;
    int state;

    QThread *launcher_thread;
    MmAppLauncher *launcher;
};

#endif // MM_KEY_EXECUTER_H
