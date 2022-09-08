#include "mm_nt_user.h"
#include "mm_win32.h"
#include <tchar.h>

HWND* _Gui_BuildWindowList(NtUserBuildHwndList pNtUserBuildHwndList,
                           HDESK in_hDesk, HWND in_hWnd,
                           BOOL in_EnumChildren, BOOL in_RemoveImmersive,
                           UINT in_ThreadID, INT* out_Cnt)
{
    /* locals */
    UINT  lv_Max;
    UINT  lv_Cnt;
    UINT  lv_NtStatus;
    HWND* lv_List;

    // initial size of list
    lv_Max = 512;

    // retry to get list
    for( ; ; )
    {
        // allocate list
        if( (lv_List=(HWND*)malloc(lv_Max * sizeof(HWND)))==NULL )
        {
            break;
        }

        // call the api
        lv_NtStatus = pNtUserBuildHwndList( in_hDesk, in_hWnd,
            in_EnumChildren, in_RemoveImmersive, in_ThreadID,
            lv_Max, lv_List, &lv_Cnt);

        // success?
        if( lv_NtStatus==NOERROR )
        {
            break;
        }

        // free allocated list
        free(lv_List);

        // clear
        lv_List = NULL;

        // other error then buffersize? or no increase in size?
        if( lv_NtStatus!=STATUS_BUFFER_TOO_SMALL || lv_Cnt<=lv_Max )
        {
            break;
        }

        // update max plus some extra to take changes in number of windows into account
        lv_Max = lv_Cnt + 16;
    }

    // return the count
    *out_Cnt = lv_Cnt;

    // return the list, or NULL when failed
    return lv_List;
}


/********************************************************/
/* enumerate all top level windows including metro apps */
/********************************************************/

BOOL Gui_RealEnumWindows(NtUserBuildHwndList pNtUserBuildHwndList)
{
    /* locals */
    INT   lv_Cnt;
    HWND  lv_hWnd;
    BOOL  lv_Result;
    HWND  lv_hFirstWnd;
    HWND  lv_hDeskWnd;
    HWND* lv_List;
    QString name;

    // no error yet
    lv_Result = TRUE;

    // first try api to get full window list including immersive/metro apps
    lv_List = _Gui_BuildWindowList(pNtUserBuildHwndList, 0, 0, 0, 0, 0, &lv_Cnt);

    // success?
    if( lv_List )
    {
        // loop through list
        while( lv_Cnt-->0 && lv_Result )
        {
            // get handle
            lv_hWnd = lv_List[lv_Cnt];

            // filter out the invalid entry (0x00000001) then call the callback
            if( IsWindow(lv_hWnd) )
            {
                WCHAR class_name[200];
//                char sag_name[200];
                int ret_class = GetClassName(lv_hWnd, class_name, 200);
//                _tprintf(_T("class_name %s\n"), class_name);
//                if( ret_class!=0 )
//                {
//                    sprintf(sag_name, "class_name %s ", class_name);
//                }
                int is_alt_tab = IsAltTabWindow(lv_hWnd);
                name = mm_getPNameA(lv_hWnd);
                qDebug() << ">>" << name << "is_alt_tab:" << is_alt_tab << QString::fromWCharArray(class_name);
            }
        }

        // free the list
        free(lv_List);
    }
    else
    {
        // get desktop window, this is equivalent to specifying NULL as hwndParent
        lv_hDeskWnd = GetDesktopWindow();

        // fallback to using FindWindowEx, get first top-level window
        lv_hFirstWnd = FindWindowEx(lv_hDeskWnd, 0, 0, 0);

        // init the enumeration
        lv_Cnt = 0;
        lv_hWnd = lv_hFirstWnd;

        // loop through windows found
        // - since 2012 the EnumWindows API in windows has a problem (on purpose by MS)
        //   that it does not return all windows (no metro apps, no start menu etc)
        // - luckally the FindWindowEx() still is clean and working
        while( lv_hWnd && lv_Result )
        {
            // call the callback
//            name = mm_getPNameA(lv_hWnd);
//            qDebug() << "success:" << name;

            // get next window
            lv_hWnd = FindWindowEx(lv_hDeskWnd, lv_hWnd, 0, 0);

            // protect against changes in window hierachy during enumeration
            if( lv_hWnd==lv_hFirstWnd || lv_Cnt++>10000 )
            {
                break;
            }
        }
    }

    // return the result
//    exit(0);
    return lv_Result;
}
