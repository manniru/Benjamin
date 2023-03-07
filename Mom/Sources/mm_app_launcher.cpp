#include "mm_app_launcher.h"
#include <QFileInfo>
#include <QThread>

MmAppLauncher::MmAppLauncher(MmVirt *vi, QObject *parent)
                            : QObject(parent)
{
    virt = vi;
    lua = new MmLua;
}

MmAppLauncher::~MmAppLauncher()
{
    ;
}

MmApplication MmAppLauncher::getApplication(QString shortcut_name,
                                            QString win_title)
{
    MmApplication app;
    app.shortcut_name = shortcut_name;
    app.win_title = win_title;
    shortcut_name += ".lnk";
    mm_getLinkPath(shortcut_name, &app);
    QFileInfo fi(app.exe_path);
    app.exe_name = fi.completeBaseName();
    app.hwnd = mm_getHWND(&app);
    return app;
}

void MmAppLauncher::focusOpen(QString shortcut, int desktop_id)
{
    MmApplication app = getApplication(shortcut, "");

    if( app.hwnd )
    {
        focus(app.hwnd);
    }
    else
    {
        if( desktop_id!=-1 )
        {
            qDebug() << "1 focusOpen" << desktop_id << virt;
            virt->setDesktop(desktop_id);
            qDebug() << "2 NAKONID";
            QThread::msleep(5000);
            qDebug() << "3 NAKONID";
        }
        mm_launchApp(shortcut);
    }
}

void MmAppLauncher::focus(HWND hwnd)
{
    DWORD windowThreadProcessId = GetWindowThreadProcessId(GetForegroundWindow(),LPDWORD(0));
    DWORD currentThreadId = GetCurrentThreadId();
    DWORD CONST_SW_SHOW = 5;
    AttachThreadInput(windowThreadProcessId, currentThreadId, true);
    BringWindowToTop(hwnd);
    ShowWindow(hwnd, CONST_SW_SHOW);
    AttachThreadInput(windowThreadProcessId,currentThreadId, false);

    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
    SetFocus(hwnd);
}

void MmAppLauncher::openFirefox()
{
    MmApplication app = getApplication("Firefox", "");

    if( app.hwnd )
    {
        focus(app.hwnd);
    }
    else
    {
        lua->run(); // lua fix ask password bug
        virt->setDesktop(4);
        QThread::msleep(200);
        mm_launchApp("Firefox", "--remote-debugging-port");
    }
}
