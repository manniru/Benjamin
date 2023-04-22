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
        if( key->emul_mode )
        {
            qDebug() << "emul mode, suppress_r:" << key->is_mom;
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
            int ret = key->procReleaseKey(key_code);

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
    state = 0;
    emul_mode = 0;
    is_mom = 0;
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
    if( key_code==VK_LWIN ||
        key_code==VK_RWIN )
    {
        addPressKey(key_code);
        return 1;
    }
    else if( key_code==VK_APPS )
    {
        addPressKey(VK_LWIN);
        return 1;
    }
    else if( key_code==VK_LSHIFT   ||
             key_code==VK_RSHIFT   ||
             key_code==VK_LCONTROL ||
             key_code==VK_LCONTROL )
    {
        if( pk_buf.length() )
        {
            addPressKey(key_code);
            return 1;
        }
    }
    else
    {
        if( pk_buf.length() )
        {
            qDebug() << "execWinKey" << key_code;
            MmKbState state = getState();
            int ret = exec->execWinKey(key_code, state);
            if( ret )
            {
                is_mom = 1;
            }
            else
            { // press all captured keys
                qDebug() << "fakePress";
                fakePress(0);
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

int MmKeyboard::procReleaseKey(int key_code)
{
    qDebug() << "procReleaseKey key code:" << key_code
             << "suppress_r:"              << is_mom;

    if( is_mom==1 ) // a mom shortcut captured
    {
        if( key_code==VK_APPS )
        {
            key_code = VK_LWIN;
        }
        int is_sup = isSuppressed(key_code);

        if( is_sup )
        {
            qDebug() << "suppress release";
            return 1;
        }
    }
    else if( pk_buf.length() )
    {
        if( key_code==VK_APPS )
        {
            pk_buf[0] = VK_APPS;
        }
        int exec_start = 0;
        if( key_code==VK_LWIN ||
            key_code==VK_RWIN  )
        {
            exec_start = 1;
        }
        fakePress(exec_start);
    }

    return 0;
}

//BOOL AppBar_SetSide(HWND hwnd)
void MmKeyboard::SetSide()
{

}

void MmKeyboard::fakePress(int exec_start)
{
    emul_mode = 1;

    int len = pk_buf.length();
    for( int i=0 ; i<len ; i++ )
    {
        int key = pk_buf[i];
        e_key->pressKey(key);
    }

    if( len==1 && exec_start )
    {
        e_key->releaseKey(pk_buf[0]);
    }

    pk_buf.clear();
    emul_mode = 0;
}

void MmKeyboard::fakeRelease(int key_code)
{
    qDebug() << "key->supress_r=1";
    key->key_buf.push_back(key_code);
    key->timer->start(2);
    qDebug() << "key->supress_r=0";
}

// This function will prevent adding duplicate item
// to the "press key buffer" as holiding a key down
// result in duplicates
void MmKeyboard::addPressKey(int key_code)
{
    int len = pk_buf.length();

    for( int i=0 ; i<len ; i++ )
    {
        if( key_code==pk_buf[i] )
        {
            return;
        }
    }

    qDebug() << "addPressKey" << key_code
             << "is_mom:"     << is_mom;
    pk_buf.push_back(key_code);
}

// Release Suppressed
int MmKeyboard::isSuppressed(int key_code)
{
    int len = pk_buf.length();

    if( len==0 )
    {
        return 0;
    }

    qDebug() << "isSuppressed"
             << len;

    for( int i=0 ; i<len ; i++ )
    {
        if( key_code==pk_buf[i] )
        {
            pk_buf.remove(i);
            if( pk_buf.length()==0 )
            {
                qDebug() << "is_mom RESET";
                is_mom = 0;
            }
            return 1;
        }
    }

    return 0;
}

MmKbState MmKeyboard::getState()
{
    MmKbState state;
    int len = pk_buf.length();

    for( int i=0 ; i<len ; i++ )
    {
        if( pk_buf[i]==VK_LSHIFT ||
            pk_buf[i]==VK_RSHIFT )
        {
            state.shift_down = 1;
        }
        else if( pk_buf[i]==VK_LCONTROL ||
                 pk_buf[i]==VK_RCONTROL )
        {
            state.ctrl_down = 1;
        }
//        else if( pk_buf[i]==VK_ ||
//                 pk_buf[i]==VK_RSHIFT )
//        {
//            state.shift_down = 1;
//        }
    }

    return state;
}
