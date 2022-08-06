#include "mm_virt.h"
#include <initguid.h>
#include <windows.h>
#include <inspectable.h> // IInspectable, HSTRING, TrustLevel
#include <shobjidl.h> // IObjectArray, ObjectArray, IVirtualDesktopManager, VirtualDesktopManager
#include <strsafe.h> // StringCbPrintf
#include <QDebug>

MmVirt::MmVirt(QObject *parent): QObject(parent)
{
    pDesktopManager = NULL;
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    IServiceProvider* pServiceProvider = NULL;
    HRESULT hr = CoCreateInstance(
                        CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER,
                        __uuidof(IServiceProvider), (PVOID*)&pServiceProvider);

    hr = pServiceProvider->QueryService(CLSID_VirtualDesktopAPI_Unknown,
                                        IID_IVirtualDesktopManagerInternal,
                                        (void **)&pDesktopManager);

    pServiceProvider->Release();

    updateGUID();
}

MmVirt::~MmVirt()
{
    pDesktopManager->Release();
}

void MmVirt::updateGUID()
{
    IObjectArray *desktops;
    IVirtualDesktop *nextDesktop;

    pDesktopManager->GetDesktops(&desktops);
    UINT count;
    desktops->GetCount(&count);

    for( unsigned int i=0 ; i<count ; i++ )
    {
        desktops->GetAt(i, UUID_IVirtualDesktop, (void**)&nextDesktop);

        GUID buffer;
        nextDesktop->GetID(&buffer);
        vd_guids << buffer;

        nextDesktop->Release();
    }

    desktops->Release();
}

void MmVirt::setDesktop(int id)
{
    IObjectArray *desktops;
    IVirtualDesktop *nextDesktop;

    HRESULT hr = pDesktopManager->GetDesktops(&desktops);

    desktops->GetAt(id, UUID_IVirtualDesktop, (void**)&nextDesktop);
    desktops->Release();

    hr = pDesktopManager->SwitchDesktop(nextDesktop);
    nextDesktop->Release();
}

int MmVirt::getCurrDesktop()
{
    IVirtualDesktop *currDesktop;

    pDesktopManager->GetCurrentDesktop(&currDesktop);
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
