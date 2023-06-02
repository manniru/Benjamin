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
        mm_focusWindow(app.hwnd);
    }
    else
    {
        if( desktop_id!=-1 )
        {
            virt->setDesktop(desktop_id);
            QThread::msleep(50);
        }
        mm_launchLnk(shortcut);
    }
}

void MmAppLauncher::openFirefox()
{
    MmApplication app = getApplication("Firefox", "");

    if( app.hwnd )
    {
        mm_focusWindow(app.hwnd);
    }
    else
    {
        lua->run(); // lua fix ask password bug
        virt->setDesktop(3);
        QThread::msleep(200);
        mm_launchLnk("Firefox", "--remote-debugging-port");
    }
}

void MmAppLauncher::launchCMD()
{
    MmApplication app;
    app.exe_path = "cmd";
    app.working_dir = "C:\\Arch";
    mm_launchApp(&app);
}
