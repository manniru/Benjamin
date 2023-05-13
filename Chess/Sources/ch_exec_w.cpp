#include "ch_exec_w.h"
#include <QWindow>

ChExecW::ChExecW(QObject *ui,
                 QObject *parent) : QObject(parent)
{
    root = ui;
    mon  = new ChMonitor;

    QWindow *window = qobject_cast<QWindow *>(ui);
    hWnd = (HWND)(window->winId());
}

ChExecW::~ChExecW()
{
    ;
}

void ChExecW::activateWindow()
{
    bool forground_en = SetForegroundWindow(hWnd);
    HWND last_active = SetActiveWindow(hWnd);
    HWND focus_ret = SetFocus(hWnd);

    bool pos_ret = SetWindowPos(hWnd, HWND_TOPMOST,
                                0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

//    qDebug() << "SetFocus" << focus_ret
//             << "SetWindowPos" << pos_ret
//             << "SetForegroundWindow" << forground_en
//             << "SetActiveWindow" << last_active;

    QThread::msleep(50);
    SetCursorPos(mid_x, mid_y);
    QThread::msleep(50);
    sendLeftKey();
}

void ChExecW::setLanguage()
{
    char layout_id[200];
    GetKeyboardLayoutNameA(layout_id);
    QString layout_idq = layout_id;

    qDebug() << "Language" << layout_idq;

    if( layout_idq=="00000429" )
    {
        qDebug() << "Changing to english";
        ActivateKeyboardLayout((HKL)HKL_NEXT,
                               KLF_SETFORPROCESS);
    }
}

void ChExecW::updateScreen(QString cmd)
{
    int width = 0;
    int height = 0;
    int x = 0;
    int y = 0;
    if( cmd=="comment" )
    {
        width  = mon->secondary.width;
        height = mon->secondary.height;
        x = mon->secondary.x;
        y = mon->secondary.y;
    }
    else
    {
        width  = mon->primary.width;
        height = mon->primary.height;
        x = mon->primary.x;
        y = mon->primary.y;
    }

    QQmlProperty::write(root, "width", width);
    QQmlProperty::write(root, "height", height+40);
    QQmlProperty::write(root, "x", x);
    QQmlProperty::write(root, "y", y);

    mid_x = x + width/2;
    mid_y = y + height/2;
//    qDebug() << "width" << width << "height" << height
//             << "x" << x << "y" << y;
}

void ChExecW::sendLeftKey()
{
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = 0;
    input.mi.dy = 0;
    input.mi.dwFlags = (MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP);
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = NULL;
    input.mi.time = 0;
    SendInput(1,&input,sizeof(INPUT));
}

void ChExecW::sendRightKey()
{
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = 0;
    input.mi.dy = 0;
    input.mi.dwFlags = (MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_RIGHTUP);
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = NULL;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void ChExecW::sendMiddleKey()
{
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = 0;
    input.mi.dy = 0;
    input.mi.dwFlags = (MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MIDDLEUP);
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = NULL;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}
