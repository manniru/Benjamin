#ifndef MM_KEYBOARD_H
#define MM_KEYBOARD_H

#include <QObject>
#include <QString>
#include <QVector>
#include <Windows.h>
#include "mm_virt.h"
#include "mm_api.h"
#include "mm_lua.h"

class MmKeyboard : public QObject
{
    Q_OBJECT
public:
    explicit MmKeyboard(MmVirt *vi,
                        QObject *parent = nullptr);
    ~MmKeyboard();

    int procPressKey(int key_code);

public slots:
    void procState();

private:
    int procVirtKey(int key_code);
    int procWinKey(int key_code);
    void SetSide();
    void goToSleep();

    HHOOK hHook = NULL;
    MmVirt *virt;
    QTimer *timer;
    MmLua *lua;
    int state;
};

void mm_setKeyboard(MmKeyboard *val);

#endif // MM_KEYBOARD_H
