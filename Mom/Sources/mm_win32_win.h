#ifndef MM_WIN32_WIN_H
#define MM_WIN32_WIN_H

#include <QStringList>

#include <dwmapi.h>
#include <tlhelp32.h> // to get pid
#include <psapi.h> // For access to GetModuleFileNameEx
#include "Windows.h"
#include "mm_win32.h"

typedef struct MmWindow
{
    // Verify Clear On Each Enumeration To
    int  verify; //verify hwnd still exist
    int  type;
    QString title;
    QString pname;
    HWND hWnd;
}MmWindow;

class MmWin32Win
{
public:
    MmWin32Win();
    void updateActiveWindow();
    void insertWindow(MmWindow win);

    QVector<MmWindow> windows;
    MmWindow win_active;

private:
};

void re_AddHwnd(HWND hwnd, MmWin32Win *thread_w);
void reRunThread();

#endif // MM_WIN32_WIN_H
