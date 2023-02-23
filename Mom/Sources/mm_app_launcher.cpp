#include "mm_app_launcher.h"
#include <QFileInfo>

MmAppLauncher::MmAppLauncher(QObject *parent) : QObject(parent)
{

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
    app.exe_path = mm_getLinkPath(shortcut_name);
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
        SetForegroundWindow(app.hwnd);
        SetActiveWindow(app.hwnd);
        SetFocus(app.hwnd);
    }
    else
    {
        mm_launchApp(shortcut);
    }
}
