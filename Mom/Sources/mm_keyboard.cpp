#include "mm_keyboard.h"
#include <QDebug>
#include <QThread>
#include <powrprof.h>

MmKeyboard *key;

void mm_setKeyboard(MmKeyboard *val)
{
    key = val;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if( nCode >= 0 )
    {
        KBDLLHOOKSTRUCT *keyinfo = (KBDLLHOOKSTRUCT *)lParam;
        if( wParam==WM_KEYDOWN )
        {
            int ret = key->procPressKey(keyinfo->vkCode);
            if( ret )
            {
                return ret;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

MmKeyboard::MmKeyboard(MmVirt *vi, QObject *parent) : QObject(parent)
{
    virt  = vi;
    state = 0;
//    int thread_id = GetCurrentThreadId();
//    hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, 0, thread_id);
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
    if( hHook==NULL )
    {
        qDebug() << "[!]set hook fail";
        return;
    }
//    vi->setDesktop(4);
    qDebug() << "[+]hHOOK";

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()),
            this, SLOT(procState()));
    timer->start(2);


    lua = new MmLua();

}

MmKeyboard::~MmKeyboard()
{
    if( hHook )
    {
        UnhookWindowsHookEx(hHook);
        hHook = NULL;
    }
}

int MmKeyboard::procWinKey(int key_code)
{
    if( key_code>='1' &&
        key_code<='6' )
    {
        int id = key_code-'0';
        state = id;
        return 1;
    }
    else if( key_code=='A' )
    {
        state = 'a';
        return 1;
    }
    else if( key_code=='D' )
    {
        state = 'd';
        return 1;
    }
    else if( key_code=='P' )
    {
        mm_launchApp("Qt Creator\\Qt Creator 4.15.1 (Community)");
        return 1;
    }
    else if( key_code=='S' )
    {
        mm_launchApp("Spotify");
        return 1;
    }
    else if( key_code=='T' )
    {
        mm_launchApp("Telegram Desktop\\Telegram");
        return 1;
    }
    else if( key_code=='W' )
    {
        mm_launchApp("GitKraken\\GitKraken");
        return 1;
    }
    else if( key_code=='Y' )
    {
        mm_launchApp("Visual Studio Code\\Visual Studio Code");
        return 1;
    }

    else if( key_code==VK_OEM_7 ) // Quote '
    {
        state = virt->last_desktop;
        return 1;
    }
    return 0;
}

int MmKeyboard::procPressKey(int key_code)
{
    if( key_code!=VK_LWIN &&
        key_code!=VK_RWIN )
    {
        int wl_state = GetKeyState(VK_LWIN);
        int wr_state = GetKeyState(VK_RWIN);
//        qDebug() << "key" << key_code
//                 << wr_state;
        if( wl_state<0 ||
            wr_state<0 )
        {
            int ret = procWinKey(key_code);
            return ret;
        }
        else if( key_code==VK_PAUSE )
        {
            SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
            return 1;
        }
        else if( key_code==VK_CANCEL )
        {
            // go to hibernate
            int hibernate = 0;
            int force = 0;
            int disable_wake = 0;
            SetSuspendState(hibernate, force,
                            disable_wake);
            return 1;
        }
    }

    return 0;
}

void MmKeyboard::procState()
{
    if( state=='d' )
    {
        lua->run(); // lua fix ask password bug
        QThread::msleep(2000);
        mm_launchApp("Firefox", "--remote-debugging-port");
    }
    else if( state=='a' )
    {
        mm_launchScript(RE_WINSCR_DIR"\\git_date.cmd");
    }
    else if( state )
    {
        virt->setDesktop(state-1);
    }
    state = 0;
}

int MmKeyboard::procVirtKey(int key_code)
{
    if( key_code>='1' &&
        key_code<='6' )
    {
        int id = key_code-'0';
        state = id;
        return 1;
    }
    return 0;
}

//BOOL AppBar_SetSide(HWND hwnd)
void MmKeyboard::SetSide()
{

}
