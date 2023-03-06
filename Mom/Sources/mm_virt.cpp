#include "mm_virt.h"
#include <initguid.h>
#include <windows.h>
#include <inspectable.h> // IInspectable, HSTRING, TrustLevel
#include <shobjidl.h> // IObjectArray, ObjectArray, IVirtualDesktopManager, VirtualDesktopManager
#include <strsafe.h> // StringCbPrintf
#include <QDebug>
#include <QThread>
#include "mm_win32_win.h"

MmVirt::MmVirt(QObject *parent): QObject(parent)
{
    pDesktopManagerInt = NULL;
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    IServiceProvider* pServiceProvider = NULL;
    HRESULT hr = CoCreateInstance(
                        CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER,
                        __uuidof(IServiceProvider), (PVOID*)&pServiceProvider); 

    hr = pServiceProvider->QueryService(IID_IVirtualDesktopManager,
                                        IID_IVirtualDesktopManager,
                                        (void **)&pDesktopManager);
    if( hr )
    {
        qDebug() << "VirtualDesktopManager Failed" << hr;
    }

    hr = pServiceProvider->QueryService(IID_IApplicationViewCollection,
                                        IID_IApplicationViewCollection,
                                        (void **)&pAppViewCol);
    if( hr )
    {
        qDebug() << "IApplicationViewCollection Failed" << hr;
    }

    hr = pServiceProvider->QueryService(CLSID_VirtualDesktopAPI_Unknown,
                                        IID_IVirtualDesktopManagerInternal,
                                        (void **)&pDesktopManagerInt);
    pServiceProvider->Release();

    updateGUID();

    current_desktop = getCurrDesktop();
    last_desktop = -1;

    win_lister = new MmWin32Win();
}

MmVirt::~MmVirt()
{
    pDesktopManagerInt->Release();
}

void MmVirt::updateGUID()
{
    IObjectArray *desktops;
    IVirtualDesktop *c_desktop;

    pDesktopManagerInt->GetDesktops(&desktops);
    UINT count;
    desktops->GetCount(&count);

    for( unsigned int i=0 ; i<count ; i++ )
    {
        desktops->GetAt(i, UUID_IVirtualDesktop, (void**)&c_desktop);

        GUID buffer;
        c_desktop->GetID(&buffer);
        vd_guids << buffer;
        vd_desks << c_desktop;
    }

    desktops->Release();
}

void MmVirt::setDesktop(int id)
{
    if( id<vd_desks.length() && id>=0 )
    {
        current_desktop = getCurrDesktop();
        if( current_desktop==id+1 )
        {
            return;
        }
        int res = pDesktopManagerInt->SwitchDesktop(vd_desks[id]);
//        qDebug() << "setDesktop" << res;
        setFocus();
        last_desktop = current_desktop;
        current_desktop = id + 1;
    }
}

void MmVirt::moveToDesktop(int id)
{
    HWND hwnd = GetForegroundWindow();
    IApplicationView *view;
    HRESULT res = pAppViewCol->GetViewForHwnd(hwnd, &view);

    qDebug() << "GetViewForHwnd" << QString::number(res, 16);
    qDebug() << "GetViewForHwnd" << GetLastError();

    res = pDesktopManagerInt->MoveViewToDesktop(view, vd_desks[id]);


    qDebug() << "moveToDesktop" << QString::number(res, 16);
    qDebug() << "moveToDesktop" << GetLastError();
}

int MmVirt::isCurrentActive()
{
    win_lister->update();

    int win_len = win_lister->wins.length();
    for( int i=0 ; i<win_len ; i++ )
    {
        MmWindow win = win_lister->wins[i];
        HWND hwnd = win.hWnd;
        BOOL ret;
        pDesktopManager->IsWindowOnCurrentVirtualDesktop(hwnd, &ret);
        if( ret )
        {
            if( win_lister->win_active.hWnd==hwnd )
            {
                qDebug() << "New Window" << win.title;
                return true;
            }
        }
    }
    return false;
}

void MmVirt::setFocus()
{
    QThread::msleep(100);

    if( isCurrentActive()==0 )
    {
        pressKey(VK_LMENU); //ALT
        sendKey(VK_TAB);
        releaseKey(VK_LMENU);
    }
}

int MmVirt::getCurrDesktop()
{
    IVirtualDesktop *currDesktop;

    pDesktopManagerInt->GetCurrentDesktop(&currDesktop);
    GUID curr_DesktopGUID;
    currDesktop->GetID(&curr_DesktopGUID);
    currDesktop->Release();

    for( int i=0 ; i<vd_guids.length() ; i++ )
    {
        if( curr_DesktopGUID==vd_guids[i] )
        {
            return i+1;
        }
    }

    return -1;
}

// this function doesnt work
int MmVirt::getDesktop(HWND hwnd)
{
    GUID desktopGUID;
    pDesktopManager->GetWindowDesktopId(hwnd, &desktopGUID);

    for( int i=0 ; i<vd_guids.length() ; i++ )
    {
        if( desktopGUID==vd_guids[i] )
        {
            return i+1;
        }
    }

    return -1;
}

void MmVirt::sendKey(int key_val)
{
    pressKey(key_val);
    releaseKey(key_val);
}

void MmVirt::pressKey(int key_val)
{
    INPUT input;
    ZeroMemory(&input, sizeof(input));

    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key_val;

    SendInput(1, &input, sizeof(INPUT));
}

void MmVirt::releaseKey(int key_val)
{
    INPUT input;
    ZeroMemory(&input, sizeof(input));

    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key_val;
    input.ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &input, sizeof(INPUT));
}
