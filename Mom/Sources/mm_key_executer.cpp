#include "mm_key_executer.h"
#include "mm_api.h"
#include <QDebug>
#include <QThread>

MmKeyExec::MmKeyExec(MmVirt *vi, QObject *parent) : QObject(parent)
{
    state = 0;
    virt = vi;

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()),
            this, SLOT(delayedExec()));
    timer->start(2);

    launcher = new MmAppLauncher(vi);
}

MmKeyExec::~MmKeyExec()
{
    ;
}

// delayed exec introduced because virtual desktop
// is sensetive to the thread it's called from
void MmKeyExec::delayedExec()
{
    if( state=='d' )
    {
        launcher->openFirefox();
    }
    else if( state=='a' )
    {
        mm_launchScript(RE_WINSCR_DIR"\\git_date.cmd");
    }
    else if( state=='s' )
    {
        QString shortcut = "Spotify";
        launcher->focusOpen(shortcut, 4);
    }
    else if( state=='t' )
    {
        QString shortcut = "Telegram Desktop\\Telegram";
        launcher->focusOpen(shortcut, 3);
    }
    else if( state=='w' )
    {
        QString shortcut = "GitKraken\\GitKraken";
        launcher->focusOpen(shortcut, 4);
    }
    else if( state=='y' )
    {
        QString shortcut = "Visual Studio Code\\Visual Studio Code";
        launcher->focusOpen(shortcut);
    }
    else if( state>0 && state<10 )
    {
        virt->setDesktop(state-1);
    }
    else if( state ) // Super+Shift+#
    {
        int desktop_id = state-10;
        virt->moveToDesktop(desktop_id-1);
    }
    state = 0;
}

int MmKeyExec::execWinNum(int key_code)
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

int MmKeyExec::execWinKey(int key_code, MmKbState st)
{
    if( st.shift_down )
    {
        return execShiftWin(key_code);
    }

    QString shortcut;

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
        shortcut = "Qt Creator\\Qt Creator 4.15.1 (Community)";
        launcher->focusOpen(shortcut);

        return 1;
    }
    else if( key_code=='S' ) // Spotify
    {
        state = 's';
        return 1;
    }
    else if( key_code=='T' )
    {
        state = 't';
        return 1;
    }
    else if( key_code=='W' )
    {
        state = 'w';
        return 1;
    }
    else if( key_code=='Y' )
    {
        state = 'y';
        return 1;
    }
    else if( key_code==VK_OEM_7 ) // Quote '
    {
        state = virt->last_desktop;
        return 1;
    }
    return 0;
}

int MmKeyExec::execShiftWin(int key_code)
{
    if( key_code>='1' &&
        key_code<='6' )
    {
        int id = key_code-'0';
        state = id+10;
        return 1;
    }

    return 0;
}

void MmKeyExec::goToSleep(int *emul_mode)
{
    virt->pressKey(VK_LWIN);
    virt->sendKey('X');
    Sleep(200);
    virt->releaseKey(VK_LWIN);
    Sleep(200);
    virt->sendKey('U');
    Sleep(200);
    *emul_mode = 0;
    virt->sendKey('S');
}
