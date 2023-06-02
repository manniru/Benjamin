#ifndef MM_API_H
#define MM_API_H

#include <QString>
#include <Windows.h>
#include "mm_win32.h"

void mm_launchApp(MmApplication *app, QString arg="");
void mm_launchScript(QString path, QString arg="");
void mm_launchLnk(QString app_name, QString arg="");
void mm_getLinkPath(QString path, MmApplication *app);
void mm_getLinkPathUser(QString path, MmApplication *app);
void mm_getLinkPathAll(QString path, MmApplication *app);
HRESULT mm_ResolveIt(LPCSTR lnk_path, MmApplication *app);
void mm_closeWindow();
void mm_focusWindow(HWND hwnd);
int  mm_getWindowOpacity(HWND hwnd);

#endif // MM_API_H
