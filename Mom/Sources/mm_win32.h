#ifndef MM_WIN32_H
#define MM_WIN32_H

#include <QString>
#include <windows.h>

#define MM_DELAY_CLICK 200
#define MM_MOUSE_LKEY 1
#define MM_MOUSE_MKEY 2
#define MM_MOUSE_RKEY 3

#define MAX_TITLE_LEN 200

typedef struct MmApplication
{
    QString shortcut_name;
    QString exe_name;
    QString exe_path;
    QString win_title;
    DWORD pid = 0;
    QString pname;
    int workspace;
    HWND hwnd = 0;
}MmApplication;

void mm_sendMouseKey(int key);
void mm_sendMouseFlag(int flag);
QString mm_getPNameA(HWND hwnd);
QString mm_getPName(long pid);
long mm_getPid(HWND hWnd);
BOOL IsAltTabWindow(HWND hwnd);
HWND mm_getHWND(MmApplication *app);
HWND mm_getFocusedHWND(MmApplication *app);
QString mm_getWinTitle(HWND hwnd);

#endif // MM_WIN32_H
