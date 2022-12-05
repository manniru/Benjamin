#include "mm_key_executer.h"
#include "mm_api.h"
#include <QDebug>
#include <QThread>

MmKeyExec::MmKeyExec(MmVirt *vi, QObject *parent) : QObject(parent)
{
    state = 0;
    virt = vi;

    lua = new MmLua;
    timer = new QTimer;
    connect(timer, SIGNAL(timeout()),
            this, SLOT(delayedExec()));
    timer->start(2);
}

MmKeyExec::~MmKeyExec()
{
    ;
}

void MmKeyExec::delayedExec()
{
    if( state=='d' )
    {
        lua->run(); // lua fix ask password bug
        QThread::msleep(2000);
        mm_launchApp("Firefox", "--remote-debugging-port");
    }
    else if( state=='a' )
    {
        qDebug() << "PAKAM NAKONID";
        mm_launchScript(RE_WINSCR_DIR"\\git_date.cmd");
    }
    else if( state )
    {
        virt->setDesktop(state-1);
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

int MmKeyExec::execWinKey(int key_code)
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

void MmKeyExec::goToSleep()
{
    virt->pressKey(VK_LWIN);
    virt->sendKey('X');
    Sleep(200);
    virt->releaseKey(VK_LWIN);
    Sleep(200);
    virt->sendKey('U');
    Sleep(200);
    virt->sendKey('S');
}
