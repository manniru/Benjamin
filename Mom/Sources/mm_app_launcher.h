#ifndef MM_APP_LAUNCHER_H
#define MM_APP_LAUNCHER_H

#include <Windows.h>
#include <QtDebug>
#include "mm_api.h"
#include "mm_lua.h"
#include "mm_virt.h"
#include "mm_win32.h"

class MmAppLauncher : public QObject
{
    Q_OBJECT
public:
    explicit MmAppLauncher(MmVirt *vi, QObject *parent = nullptr);
    ~MmAppLauncher();

    void focusOpen(QString shortcut, int desktop_id=-1);
    void openFirefox();

private:
    MmApplication getApplication(QString shortcut_name, QString win_title);
    void focus(HWND hwnd);

    MmLua  *lua;
    MmVirt *virt;
};

#endif // MM_APP_LAUNCHER_H
