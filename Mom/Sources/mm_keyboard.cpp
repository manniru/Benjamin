#include "mm_keyboard.h"
#include "mm_win32.h"
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
        int key_code = keyinfo->vkCode;
        if( wParam==WM_KEYDOWN )
        {
            int ret = key->procPressKey(key_code);
//            if( key_code==VK_LWIN ||
//                key_code==VK_RWIN )
//            {
//                return 1;
//            }
            if( ret )
            {
                return ret;
            }
        }
        else if( wParam==WM_KEYUP )
        {
            if( key_code==VK_LWIN ||
                key_code==VK_RWIN )
            {
                if( key->supress_r )
                {
                    key->supress_r = 0;
                    qDebug() << "key->supress_r";
                    return 1;
                }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

MmKeyboard::MmKeyboard(MmVirt *vi, QObject *parent) : QObject(parent)
{
    state = 0;
    win_p = 0;
    supress_r = 0;

    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
    if( hHook==NULL )
    {
        qDebug() << "[!]set hook fail";
        return;
    }

    e_key = new MmKeyEmulator;
    exec  = new MmKeyExec(vi);
}

MmKeyboard::~MmKeyboard()
{
    if( hHook )
    {
        UnhookWindowsHookEx(hHook);
        hHook = NULL;
    }
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
            int sl_state = GetKeyState(VK_LSHIFT);
            int sr_state = GetKeyState(VK_RSHIFT);

            if( sl_state<0 ||
                sr_state<0 ) // if shift is pressed
            {                // ignore
                return 0;
            }

            int ret = exec->execWinKey(key_code);
            if( ret )
            {
                supress_r = 1;
                qDebug() << "supress_r";
            }
            return ret;
        }
        else if( key_code==VK_PAUSE )
        {
//            SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
            exec->goToSleep();
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

//BOOL AppBar_SetSide(HWND hwnd)
void MmKeyboard::SetSide()
{

}
