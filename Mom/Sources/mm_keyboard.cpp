#include "mm_keyboard.h"
#include "mm_win32.h"
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
        qDebug() << "emul mode:"   << key->emul_mode
                  << "suppress_r:" << key->suppress_r;
        if( key->emul_mode )
        {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        KBDLLHOOKSTRUCT *keyinfo = (KBDLLHOOKSTRUCT *)lParam;
        int key_code = keyinfo->vkCode;
        if( wParam==WM_KEYDOWN )
        {
            int ret = key->procPressKey(key_code);

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
                qDebug() << "key win up";
                if( key->suppress_r==1 ) // only win press
                {
                    qDebug() << "key->supress_r=1";
                    key->key_buf.push_back(VK_LWIN);
                    key->suppress_r = 0;
                    key->timer->start(2);
                    qDebug() << "key->supress_r=0";
                }
                else if( key->suppress_r==2 ) // shortcut that not captured
                {
                    key->suppress_r = 0;
                }
                else if( key->suppress_r==3 ) // captured and should be suppressed
                {
                    key->suppress_r = 0;
                    return 1;
                }
                else // this would not happen
                {
                    qDebug() << "this would not happen";
                    key->suppress_r = 0;
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
    suppress_r = 0;
    virt = vi;
    timer = new QTimer;
    connect(timer, SIGNAL(timeout()),
            this, SLOT(delayedExec()));

    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
    if( hHook==NULL )
    {
        qDebug() << "[!]set hook fail";
        return;
    }

    e_key = new MmKeyEmulator;
    exec  = new MmKeyExec(vi);
    exec_thread = new QThread;
    exec->moveToThread(exec_thread);
    exec_thread->start();
}

MmKeyboard::~MmKeyboard()
{
    if( hHook )
    {
        UnhookWindowsHookEx(hHook);
        hHook = NULL;
    }
}

void MmKeyboard::delayedExec()
{
    int len = key_buf.length();
    if( len )
    {
        emul_mode = 1;
        for( int i=0 ; i<len ; i++ )
        {
            e_key->pressKey(key_buf[i]);
            QThread::msleep(5);
            qDebug() << "timer 1";
        }
        for( int i=len ; i>0 ; i-- )
        {
            e_key->releaseKey(key_buf[i-1]);
            QThread::msleep(5);
            qDebug() << "timer 2";
        }
        key_buf.clear();
        timer->stop();
        emul_mode = 0;
    }
}

int MmKeyboard::procPressKey(int key_code)
{
    qDebug() << "procPressKey key code:" << key_code
             << "suppress_r:"            << suppress_r;
    if( key_code==VK_LWIN ||
        key_code==VK_RWIN )
    {
        suppress_r = 1;
        return 1;
    }
    else if( key_code==VK_LSHIFT   ||
             key_code==VK_RSHIFT   ||
             key_code==VK_LCONTROL ||
             key_code==VK_LCONTROL )
    {
        if( suppress_r==1 )
        {
            qDebug() << "VK_RSHIFT";
            suppress_r = 2; //normal
            emul_mode = 1;
            e_key->pressKey(VK_LWIN);
            emul_mode = 0;
        }
    }
    else
    {
        qDebug() << "##key code:" << key_code
                 << "suppress_r:" << suppress_r;
        if( suppress_r==1 || suppress_r==3 )
        {
            qDebug() << "execWinKey";
            int ret = exec->execWinKey(key_code);
            if( ret )
            {
                suppress_r = 3;
                qDebug() << "supress_r = 3";
            }
            else // shortcut not captured by us
            {
                suppress_r = 2; //normal
                emul_mode = 1;
                e_key->pressKey(VK_LWIN);
                emul_mode = 0;
            }
            return ret;
        }
        else if( key_code==VK_PAUSE )
        {
//            SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
            exec->goToSleep(&emul_mode);
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
