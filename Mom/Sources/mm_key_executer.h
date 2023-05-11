#ifndef MM_KEY_EXECUTER_H
#define MM_KEY_EXECUTER_H

#include <stdio.h>
#include <Windows.h>
#include <QVector>
#include "mm_app_launcher.h"
#include "mm_virt.h"
#include "mm_sound.h"
#include "mm_win_manager.h"

typedef struct MmKbState
{
    int shift_down = 0;
    int ctrl_down  = 0;
    int alt_down   = 0;
} MmKbState;

class MmKeyExec : public QObject
{
    Q_OBJECT
public:
    explicit MmKeyExec(MmVirt *vi, MmSound *snd,
                       QObject *parent = nullptr);
    ~MmKeyExec();

    int supress_r = 0; //suprress release flag for win key
    int win_p = 0;
    int execWinKey(int key_code, MmKbState st);
    void goToSleep(int *emul_mode);

public slots:
    void delayedExec();

private:
    int execShiftWin(int key_code);

    QTimer  *timer;
    MmVirt  *virt;
    MmSound *sound;
    //cannot be QString as of the virtual num msg
    QString key_buf;

    MmAppLauncher *launcher;
    MmWinManager  *manager;
};

#endif // MM_KEY_EXECUTER_H
