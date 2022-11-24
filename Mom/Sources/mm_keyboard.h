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
    int supress_r = 0; //suprress release flag for win key
    int win_p = 0;

private:
    void SetSide();
    void goToSleep();

    HHOOK hHook = NULL;
    MmKeyExec *exec;
    MmKeyEmulator *e_key;
    int state;
};

void mm_setKeyboard(MmKeyboard *val);

#endif // MM_KEYBOARD_H
