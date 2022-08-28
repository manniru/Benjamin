#include "ch_keyboard_w.h"
#include <QThread>
#include <windows.h>

void ch_setFocus()
{
    QThread::msleep(100);

    ch_pressKey(VK_LMENU); //ALT
    ch_sendKey(VK_TAB);
    ch_releaseKey(VK_LMENU);
}

void ch_sendKey(int key_val)
{
    ch_pressKey(key_val);
    ch_releaseKey(key_val);
}

void ch_pressKey(int key_val)
{
    INPUT input;
    ZeroMemory(&input, sizeof(input));

    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key_val;

    SendInput(1, &input, sizeof(INPUT));
}

void ch_releaseKey(int key_val)
{
    INPUT input;
    ZeroMemory(&input, sizeof(input));

    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key_val;
    input.ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &input, sizeof(INPUT));
}
