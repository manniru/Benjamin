#include "ch_keyboard_w.h"
#include <QThread>
#include <QDebug>
#include <windows.h>
#include <psapi.h> // For access to GetModuleFileNameEx

void ch_returnFocus()
{
    QThread::msleep(100);

    if( ch_getActivePName()=="Chess" )
    {
        ch_pressKey(VK_LMENU); //ALT
        ch_sendKey(VK_TAB);
        ch_releaseKey(VK_LMENU);
    }
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

long reGetPid(HWND hWnd)
{
    // get allegro pid of window handle
    DWORD dwProcessId;
    GetWindowThreadProcessId(hWnd, &dwProcessId);
    if(long(dwProcessId) < 0)
    {
        qDebug() <<"Warning: couldn't get pid of allegro from window handle";
        return -1;
    }
    return dwProcessId;
}

QString ch_getActivePName()
{
    HWND hwnd = GetForegroundWindow();
    long pid = reGetPid(hwnd);;
    HANDLE processHandle = NULL;
//    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if(processHandle == NULL)
    {
        qDebug() << "Warning: couldn't get process handle from pid" << pid;
        return "";
    }

    // get name of process handle
    char path_buff[MAX_PATH];
    if(GetModuleFileNameExA(processHandle, NULL, path_buff, MAX_PATH) == 0)
    {
        qDebug() << "Error" << GetLastError() << " : Fail to get Pname of " << pid;
        return "";
    }

    // resolve short 8.3 format and get rid of ~
    char path_r[MAX_PATH];
    if( GetLongPathNameA(path_buff, path_r, MAX_PATH)==0 )
    {
        qDebug() << "Error" << GetLastError();
    }

    QString path_q = path_r; //process filename
    QStringList path_list = path_q.split("\\", QString::SkipEmptyParts);
    QString filename = path_list.last();
    filename.remove(".exe");
//    qDebug() << "path_buff" << path_q;

    return filename;
}

void ch_altTab()
{
    ch_pressKey(VK_LMENU); //ALT
    ch_sendKey(VK_TAB);
    ch_releaseKey(VK_LMENU);
}
