#ifndef MM_KEYBOARD_H
#define MM_KEYBOARD_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QTimer>
#include <Windows.h>
#include "mm_key_emulator.h"
#include "mm_key_executer.h"

// This is complex win+shortcut handler
// to prevent executing windows default shortcut like
// <win>+1, ... a low-level hook would capture all key
// press that includes <win> key in them, process them
// if the combination wasn't on the "mom" shortcut list
// it will emulate the key again, otherwise key presses captured
// and never reach the OS

class MmKeyboard : public QObject
{
    Q_OBJECT
public:
    explicit MmKeyboard(MmVirt *vi,
                        QObject *parent = nullptr);
    ~MmKeyboard();

    int procPressKey(int key_code);
    int suppress_r = 0; // suprress release flag for win key
    int emul_mode = 0; // dont interfere if we are pressing keys
                       // emulation (fake) keys are on going
    MmKeyEmulator *e_key;
    QTimer *timer;
    QVector<int> key_buf; // buffer of keycodes that
                          // need to be delayed execute

private slots:
    void delayedExec();

private:
    void SetSide();
    void goToSleep();

    HHOOK hHook = NULL;
    int state;
    MmVirt *virt;    

    QThread *exec_thread;
    MmKeyExec *exec;
};

void mm_setKeyboard(MmKeyboard *val);

#endif // MM_KEYBOARD_H
