#ifndef MM_WIN32_WIN_H
#define MM_WIN32_WIN_H

#include <QStringList>
#include <QVector>
#include <dwmapi.h>
#include <tlhelp32.h> // to get pid
#include <psapi.h> // For access to GetModuleFileNameEx
#include "Windows.h"
#include "mm_win32.h"
#include "mm_api.h"

#define MM_MINWIN_HEIGHT   50
#define MM_MINWIN_WIDTH    100
#define MM_MINWIN_OPACITY  100

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
    void update();
    void insertWindow(MmWindow win);

    QVector<MmWindow> wins;
    MmWindow win_active;

private:
};

void re_AddHwnd(HWND hwnd, MmWin32Win *thread_w);
void reRunThread();

#endif // MM_WIN32_WIN_H
