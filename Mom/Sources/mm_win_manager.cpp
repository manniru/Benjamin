#include "mm_win_manager.h"
#include "QDebug"

MmWinManager::MmWinManager(QObject *parent): QObject(parent)
{
    int x = GetSystemMetrics( SM_CXSCREEN );
    int y = GetSystemMetrics( SM_CYSCREEN );

    qDebug() << "x" << x
             << "y" << y;
}

MmWinManager::~MmWinManager()
{
    ;
}

void MmWinManager::restore()
{
    HWND hwnd = GetForegroundWindow();
    PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
}

void MmWinManager::maximise()
{
    HWND hwnd = GetForegroundWindow();
//    PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);

    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &info);
    int monitor_width = info.rcWork.right - info.rcWork.left;
    int monitor_height = info.rcWork.bottom - info.rcWork.top;

    int x = info.rcWork.left;
    int y = info.rcWork.top;
    int w = monitor_width;

    SetWindowPos(hwnd, HWND_TOP, x, y, w, monitor_height, 0);
}

void MmWinManager::minimise()
{
    HWND hwnd = GetForegroundWindow();
    PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

void MmWinManager::putLeft()
{
    HWND hwnd = GetForegroundWindow();
    PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &info);
    int monitor_width = info.rcWork.right - info.rcWork.left;
    int monitor_height = info.rcWork.bottom - info.rcWork.top;


   int x = info.rcWork.left;
   int y = info.rcWork.top;
   int w = monitor_width/2;

   SetWindowPos(hwnd, HWND_TOP, x, y, w, monitor_height, 0);
}

void MmWinManager::putRight()
{
    HWND hwnd = GetForegroundWindow();
    PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &info);
    int monitor_width = info.rcWork.right - info.rcWork.left;
    int monitor_height = info.rcWork.bottom - info.rcWork.top;


   int y = info.rcWork.top;
   int w = monitor_width/2;
   int x = info.rcWork.left + w;

   SetWindowPos(hwnd, HWND_TOP, x, y, w, monitor_height, 0);
}
