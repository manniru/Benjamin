#ifndef MM_NT_USER_H
#define MM_NT_USER_H

#include <Windows.h>
#include <QtDebug>
#include "mm_win32_const.h"

#define STATUS_BUFFER_TOO_SMALL 0xC0000023

typedef NTSTATUS(WINAPI* NtUserBuildHwndList)
(
	HDESK in_hDesk,
	HWND  in_hWndNext,
	BOOL  in_EnumChildren,
	BOOL  in_RemoveImmersive,
	DWORD in_ThreadID,
	UINT  in_Max,
	HWND* out_List,
	UINT* out_Cnt
);

HWND* _Gui_BuildWindowList
(
    NtUserBuildHwndList pNtUserBuildHwndList,
    HDESK in_hDesk,
    HWND  in_hWnd,
    BOOL  in_EnumChildren,
    BOOL  in_RemoveImmersive,
    UINT  in_ThreadID,
    INT* out_Cnt
);

BOOL Gui_RealEnumWindows(NtUserBuildHwndList pNtUserBuildHwndList);

#endif // MM_NT_USER_H
