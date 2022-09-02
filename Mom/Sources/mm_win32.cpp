#include "mm_win32.h"
#include <windows.h>
#include <QScreen>
#include <QGuiApplication>
#include <QThread>

void mm_winSleep()
{
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    int width = screen.width();
    int height = screen.height();
    int x,y;
    x = 25;
    y = height - 20;
    SetCursorPos(x, y);
    QThread::msleep(MM_DELAY_CLICK);
    mm_sendMouseKey(MM_MOUSE_LKEY);
    QThread::msleep(MM_DELAY_CLICK);
    y = height - 70;
    SetCursorPos(x, y);
    QThread::msleep(MM_DELAY_CLICK);
    mm_sendMouseKey(MM_MOUSE_LKEY);
    QThread::msleep(MM_DELAY_CLICK);
    y = height - 180;
    x = 150;
    SetCursorPos(x, y);
    QThread::msleep(MM_DELAY_CLICK);
    mm_sendMouseKey(MM_MOUSE_LKEY);
    QThread::msleep(MM_DELAY_CLICK);
    mm_sendMouseKey(MM_MOUSE_LKEY);
}

void mm_sendMouseKey(int key)
{
    if( key==MM_MOUSE_LKEY )
    {
        mm_sendMouseFlag(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP);
    }
    else if( key==MM_MOUSE_MKEY )
    {
        mm_sendMouseFlag(MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MIDDLEUP);
    }
    else if( key==MM_MOUSE_RKEY )
    {
        mm_sendMouseFlag(MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_RIGHTUP);
    }
}

void mm_sendMouseFlag(int flag)
{
    INPUT input;
    input.type=INPUT_MOUSE;
    input.mi.dx=0;
    input.mi.dy=0;
    input.mi.dwFlags=(flag);
    input.mi.mouseData=0;
    input.mi.dwExtraInfo=NULL;
    input.mi.time=0;
    SendInput(1,&input,sizeof(INPUT));
}
