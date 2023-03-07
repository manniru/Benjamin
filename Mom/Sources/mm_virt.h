#ifndef RE_WIN32_VIRT_H
#define RE_WIN32_VIRT_H

#include <objbase.h>
#include <ObjectArray.h>
#include <QVector>
#include <QObject>
#include <QTimer>
#include <windows.h>
#include "mm_config.h"
#include "mm_win32.h"
#include "mm_nt_user.h"
#include "mm_win32_win.h"
// https://github.com/senfiron/win10-virtual-desktop-switcher/tree/master/VirtualDesktopSwitcher/VirtualDesktopSwitcher
// https://github.com/chuckrector/virtual_desktopper/blob/main/virtual_desktopper.h

#ifdef _MSC_VER

#include <shobjidl_core.h>

#else

const CLSID CLSID_ImmersiveShell = {
    0xC2F03A33, 0x21F5, 0x47FA, {0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39} };

const CLSID CLSID_VirtualDesktopAPI_Unknown = {
    0xC5E0CDCA, 0x7B6E, 0x41B2, {0x9F, 0xC4, 0xD9, 0x39, 0x75, 0xCC, 0x46, 0x7B} };

const IID IID_IVirtualDesktopManagerInternal = {
    0xf31574d6, 0xb682, 0x4cdc, {0xbd, 0x56, 0x18, 0x27, 0x86, 0x0a, 0xbe, 0xc6} };

const CLSID CLSID_VirtualDesktopManager = {
    0xAA509086, 0x5CA9, 0x4C25, {0x8F, 0x95, 0x58, 0x9D, 0x3C, 0x07, 0xB4, 0x8A} };

const IID IID_IVirtualDesktopManager = {
    0xA5CD92FF, 0x29BE, 0x454C, {0x8D, 0x04, 0xD8, 0x28, 0x79, 0xFB, 0x3F, 0x1B} };

const IID IID_IApplicationViewCollection = {
      0x1841C6D7, 0x4F9D, 0x42C0, {0xAF, 0x41, 0x87, 0x47, 0x53, 0x8F, 0x10, 0xE5} };

const GUID UUID_IVirtualDesktop = {
    0xFF72FFDD, 0xBE7E, 0x43FC, {0x9C, 0x03, 0xAD, 0x81, 0x68, 0x1E, 0x88, 0xE4} };

struct IApplicationView : public IUnknown
{ };

struct IVirtualDesktop : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE IsViewVisible(
        IApplicationView *pView, int *pfVisible) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetID(
        GUID *pGuid) = 0;
};

struct IVirtualDesktopManagerInternal : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetCount(
        UINT *pCount) = 0;

    virtual HRESULT STDMETHODCALLTYPE MoveViewToDesktop(
        IApplicationView *pView,
        IVirtualDesktop *pDesktop) = 0;

    // 10240
    virtual HRESULT STDMETHODCALLTYPE CanViewMoveDesktops(
        IApplicationView *pView,
        int *pfCanViewMoveDesktops) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCurrentDesktop(
        IVirtualDesktop** desktop) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetDesktops(
        IObjectArray **ppDesktops) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetAdjacentDesktop(
        IVirtualDesktop *pDesktopReference,
        int uDirection,
        IVirtualDesktop **ppAdjacentDesktop) = 0;

    virtual HRESULT STDMETHODCALLTYPE SwitchDesktop(
        IVirtualDesktop *pDesktop) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateDesktopW(
        IVirtualDesktop **ppNewDesktop) = 0;

    virtual HRESULT STDMETHODCALLTYPE RemoveDesktop(
        IVirtualDesktop *pRemove,
        IVirtualDesktop *pFallbackDesktop) = 0;

    // 10240
    virtual HRESULT STDMETHODCALLTYPE FindDesktop(
        GUID *desktopId,
        IVirtualDesktop **ppDesktop) = 0;
};

MIDL_INTERFACE("a5cd92ff-29be-454c-8d04-d82879fb3f1b")
IVirtualDesktopManager : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
        HWND topLevelWindow,
        /* [out] */ __RPC__out BOOL *onCurrentDesktop) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
        HWND topLevelWindow,
        /* [out] */ __RPC__out GUID *desktopId) = 0;

    virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
        HWND topLevelWindow,
        REFGUID desktopId) = 0;
};

// Ignore following API's:
#define IImmersiveApplication UINT
#define IApplicationViewChangeListener UINT

struct IApplicationViewCollection : public IUnknown
{
public:
    /*** IApplicationViewCollection methods ***/
    virtual HRESULT STDMETHODCALLTYPE
            GetViews(IObjectArray**) = 0;

    virtual HRESULT STDMETHODCALLTYPE
            GetViewsByZOrder(IObjectArray**) = 0;

    virtual HRESULT STDMETHODCALLTYPE
            GetViewsByAppUserModelId(PCWSTR, IObjectArray**) = 0;

    virtual HRESULT STDMETHODCALLTYPE
            GetViewForHwnd(HWND, IApplicationView**) = 0;

    virtual HRESULT STDMETHODCALLTYPE
            GetViewForApplication(IImmersiveApplication*, IApplicationView**) = 0;

    virtual HRESULT STDMETHODCALLTYPE
            GetViewForAppUserModelId(PCWSTR, IApplicationView**) = 0;

    virtual HRESULT STDMETHODCALLTYPE
            GetViewInFocus(IApplicationView**) = 0;

    virtual HRESULT STDMETHODCALLTYPE
            RefreshCollection() = 0;

    virtual HRESULT STDMETHODCALLTYPE
            RegisterForApplicationViewChanges(
            IApplicationViewChangeListener*, DWORD*) = 0;

    virtual HRESULT STDMETHODCALLTYPE
            RegisterForApplicationViewPositionChanges(
            IApplicationViewChangeListener*, DWORD*) = 0;

    virtual HRESULT STDMETHODCALLTYPE
            UnregisterForApplicationViewChanges(DWORD) = 0;
};
#endif // _MSC_VER

class MmVirt: public QObject
{
    Q_OBJECT
public:
    explicit MmVirt(QObject *parent = nullptr);
    ~MmVirt();

    int  getCurrDesktop();
    int  getDesktop(HWND hwnd);
    void switchLastDesktop();

    int last_desktop;
    // if called from outside thread this is a better
    // way to check other wise seg fault will result
    int current_desktop;

    void sendKey(int key_val);
    void pressKey(int key_val);
    void releaseKey(int key_val);
    void setDesktop(int id);
    void moveToDesktop(int id);

private:
    void updateGUID();
    void setFocus();

    int isCurrentActive();

    QVector<GUID> vd_guids;
    QVector<HWND> vd_win[6];
    QVector<IVirtualDesktop *> vd_desks;
    IVirtualDesktopManager         *pDesktopManager;
    IApplicationViewCollection     *pAppViewCol;
    IVirtualDesktopManagerInternal *manager_int;
    MmWin32Win *win_lister;
};

#endif // RE_WIN32_VIRT_H
