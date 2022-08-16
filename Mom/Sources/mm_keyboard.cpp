#include "mm_keyboard.h"
#include <QDebug>
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
    if( state )
    {
        virt->setDesktop(state-1);
        state = 0;
    }
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
