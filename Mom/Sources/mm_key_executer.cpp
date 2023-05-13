#include "mm_key_executer.h"
#include "mm_api.h"
#include <QDebug>
#include <QThread>

MmKeyExec::MmKeyExec(MmVirt *vi, MmSound *snd,
                     QObject *parent) : QObject(parent)
{
    key_buf = "";
    virt = vi;
    sound = snd;

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()),
            this, SLOT(delayedExec()));
    timer->start(2);

    launcher = new MmAppLauncher(vi);
    manager  = new MmWinManager();
}

MmKeyExec::~MmKeyExec()
{
    ;
}

// delayed exec introduced because virtual desktop
// is sensetive to the thread it's called from
void MmKeyExec::delayedExec()
{
    if( key_buf.isEmpty() )
    {
        return;
    }
    if( key_buf=="d" )
    {
        launcher->openFirefox();
    }
    else if( key_buf=="i" )
    {
        QString shortcut = "Inkscape\\Inkscape";
        launcher->focusOpen(shortcut, 2);
    }
    else if( key_buf=="a" )
    {
        mm_launchScript(RE_WINSCR_DIR"\\git_date.cmd");
    }
    else if( key_buf=="s" )
    {
        QString shortcut = "Spotify";
        launcher->focusOpen(shortcut, 4);
    }
    else if( key_buf=="t" )
    {
        QString shortcut = "Telegram Desktop\\Telegram";
        launcher->focusOpen(shortcut, 3);
    }
    else if( key_buf=="w" )
    {
        QString shortcut = "GitKraken\\GitKraken";
        launcher->focusOpen(shortcut, 4);
    }
    else if( key_buf=="y" )
    {
        QString shortcut = "Visual Studio Code\\Visual Studio Code";
        launcher->focusOpen(shortcut);
    }
    else if( key_buf=="=" ) //enter
    {
        launcher->launchCMD();
    }
    else if( key_buf=="+" ) //up
    {
        manager->maximise();
    }
    else if( key_buf=="-" ) //down
    {
        manager->restore();
    }
    else if( key_buf=="*" ) //right
    {
        manager->putRight();
    }
    else if( key_buf=="/" ) //left
    {
        manager->putLeft();
    }
    else if( key_buf=="end" )
    {
        mm_launchLnk("Google Chrome", "https://speechnotes.co/dictate/");
    }
    else if( key_buf=="backspace" )
    {
        sound->leftClick();
    }
    else if( key_buf.contains("work") )
    {
        key_buf.remove(0, 4);
        int work_id = key_buf.toInt();
        virt->setDesktop(work_id-1);
    }
    else if( key_buf.contains("move") ) // Super+Shift+#
    {
        key_buf.remove(0, 4);
        int work_id = key_buf.toInt();
        virt->moveToDesktop(work_id-1);
    }
    key_buf = "";
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
        int id   = key_code-'0';
        key_buf  = "work";
        key_buf += QString::number(id);
        return 1;
    }
    else if( key_code=='A' )
    {
        key_buf = "a";
        return 1;
    }
    else if( key_code=='D' )
    {
        key_buf = "d";
        return 1;
    }
    else if( key_code=='I' )
    {
        key_buf = "i";
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
        key_buf = "s";
        return 1;
    }
    else if( key_code=='T' )
    {
        key_buf = "t";
        return 1;
    }
    else if( key_code=='W' )
    {
        key_buf = "w";
        return 1;
    }
    else if( key_code=='Y' )
    {
        key_buf = "y";
        return 1;
    }
    else if( key_code==VK_OEM_7 ) // Quote '
    {
        key_buf  = "work";
        key_buf += QString::number(virt->last_desktop);
        return 1;
    }
    else if( key_code==VK_RETURN ) // Enter
    {
        key_buf = "=";
        return 1;
    }
    else if( key_code==VK_UP ) // Quote '
    {
        key_buf = "+";
        return 1;
    }
    else if( key_code==VK_DOWN )
    {
        key_buf = "-";
        return 1;
    }
    else if( key_code==VK_LEFT )
    {
        key_buf = "/";
        return 1;
    }
    else if( key_code==VK_RIGHT )
    {
        key_buf = "*";
        return 1;
    }
    else if( key_code==VK_END )
    {
        key_buf = "end";
        return 1;
    }
    else if( key_code==VK_BACK ) //Backspace
    {
        key_buf = "backspace";
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
        key_buf  = "move";
        key_buf += QString::number(id);
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
