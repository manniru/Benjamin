#include "mm_app_launcher.h"
#include <QFileInfo>

MmAppLauncher::MmAppLauncher(MmVirt *vi, QObject *parent)
                            : QObject(parent)
{
    virt = vi;
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

void MmAppLauncher::focusOpen(QString shortcut)
{
    MmApplication app = getApplication(shortcut, "");

    if( app.hwnd )
    {
        focus(app.hwnd);
    }
    else
    {
        mm_launchApp(shortcut);
    }
}

void MmAppLauncher::focus(HWND hwnd)
{
    int virt_id = virt->getDesktop(hwnd);
    qDebug() << "virtId" << virt_id << hwnd;
    virt->setDesktop(virt_id);

    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
    SetFocus(hwnd);
}
