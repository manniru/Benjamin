#include "mm_win32_win.h"
#include <QDebug>

#define re_state_mode thread_data->state->i_mode

int counter = 0;
int child_num = 3;
int win_thread_debug = 0;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    char buffer[128];
    MmWin32Win *thread_w = (MmWin32Win *)lParam;
//    HWND shell_win = GetShellWindow();
//    if(!EligibleForActivation(hwnd, shell_win))
//    {
//        return TRUE;
//    }
    int written = GetWindowTextA(hwnd, buffer, 128);
    if(written && strlen(buffer) != 0 )
    {
        re_AddHwnd(hwnd, thread_w);
    }
    return TRUE;
}

//Add a new Hwnd to wins_title vector
void re_AddHwnd(HWND hwnd, MmWin32Win *thread_w)
{
    char buffer[128];
    RECT rc;

    if( IsWindowVisible(hwnd) )
    {
        int cloaked;
        DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, 4);
//        if(cloaked==0)
        if(1)
        {
            HWND shell_window = GetShellWindow();
            GetWindowRect(hwnd, &rc);
            int width = rc.right - rc.left;

            if((hwnd!=shell_window) && (width>100) ) //&& (rc.bottom>0)
            {
                int success = GetWindowTextA(hwnd, buffer, 128); //get title

                if ( success==0 )
                {
                    qDebug() << hwnd << "Failed to GetWindowTextA";
                }

                MmWindow current_win;
                current_win.hWnd = hwnd;
                current_win.title = buffer;
                current_win.pname = mm_getPName(mm_getPid(hwnd));
                current_win.verify = 1; //always new windows are verified

                thread_w->insertWindow(current_win);
            }
            else
            {
//                int success = GetWindowTextA(hwnd, buffer, 128); //get title
//                qDebug() << "----------" << buffer << rc.bottom << width;
            }
        }
    }
    else
    {
//        int success = GetWindowTextA(hwnd, buffer, 128); //get title
//        qDebug() << "not vis" << buffer << IsWindowVisible(hwnd);
    }
}

void MmWin32Win::insertWindow(MmWindow win)
{
//    qDebug() << "New Window" << win.title
//             << IsAltTabWindow(win.hWnd);
    wins.append(win);
}

MmWin32Win::MmWin32Win()
{
}

void MmWin32Win::updateActiveWindow()
{
    char buffer[128];
    win_active.hWnd = GetForegroundWindow();
    GetWindowTextA(win_active.hWnd, buffer, 128);

    win_active.title = buffer;

    if ( win_active.title.length()==0 ) //No active window
    {
        if ( wins.size()>0 )
        {
            win_active.hWnd = wins[0].hWnd; //Set last active window
        }
    }
}

void MmWin32Win::update()
{
    //Get Active Window Name
    updateActiveWindow();
    wins.clear();

    qDebug() << "-----------START-------------";
    EnumWindows(EnumWindowsProc, (LPARAM) this);
    qDebug() << "-----------END-------------";
}
