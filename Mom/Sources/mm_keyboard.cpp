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
            if( key->emul_mode )
            {
                key->emul_mode = 0;
            }
            else
            {
                int ret = key->procPressKey(key_code);

                if( ret )
                {
                    return ret;
                }
            }
        }
        else if( wParam==WM_KEYUP )
        {
            if( key_code==VK_LWIN ||
                key_code==VK_RWIN )
            {
                if( key->supress_r==0 )
                {
                    key->supress_r = 0;
                    key->emul_mode = 1;
                    key->e_key->pressKey(VK_LWIN);
                    qDebug() << "key->supress_r";
                }
                else if( key->supress_r==2 )
                {
                    key->supress_r = 0;
                }
                else
                {
                    key->supress_r = 0;
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
    emul_mode = 0;
    supress_r = 0;
    virt = vi;

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
    if( key_code==VK_LWIN ||
        key_code==VK_RWIN )
    {
        if( supress_r==0 )
        {
            supress_r = 1;
            return 1;
        }
    }
    else if( key_code==VK_LSHIFT ||
             key_code==VK_RSHIFT )
    {
        if( supress_r==1 )
        {
            supress_r = 2; //normal
            emul_mode = 1;
            e_key->pressKey(VK_LWIN);
        }
    }
    else
    {
//        qDebug() << "key" << key_code
//                 << wr_state;
        if( supress_r==1 )
        {
            int ret = exec->execWinKey(key_code);
            if( ret )
            {
                supress_r = 1;
                qDebug() << "supress_r";
            }
            else // shortcut not captured by us
            {
                supress_r = 2; //normal
                emul_mode = 1;
                e_key->pressKey(VK_LWIN);
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
