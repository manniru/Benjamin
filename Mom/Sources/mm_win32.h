#ifndef MM_WIN32_H
#define MM_WIN32_H

#include <QString>
#include <windows.h>

#define MM_DELAY_CLICK 200
#define MM_MOUSE_LKEY 1
#define MM_MOUSE_MKEY 2
#define MM_MOUSE_RKEY 3

void mm_sendMouseKey(int key);
void mm_sendMouseFlag(int flag);
QString mm_getPNameA(HWND hwnd);
QString mm_getPName(long pid);
long mm_getPid(HWND hWnd);
BOOL IsAltTabWindow(HWND hwnd);

#endif // MM_WIN32_H
