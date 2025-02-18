#include "mm_api.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <shobjidl.h>
#include <shlguid.h>
#include <QThread>
#include <psapi.h>

void mm_getLinkPath(QString path, MmApplication *app)
{
    mm_getLinkPathUser(path, app);
    if( app->exe_path.isEmpty() )
    {
        mm_getLinkPathAll(path, app);
        if( app->exe_path.isEmpty() )
        {
            qDebug() << "Error 24: cannot find shortcut"
                     << path;
        }
    }
}

void mm_getLinkPathUser(QString path, MmApplication *app)
{
    QString lnk = getenv("APPDATA");
    lnk += "\\Microsoft\\Windows\\Start Menu\\Programs\\";
    lnk += path;

    QFile file(lnk);
    if( !(QFileInfo::exists(lnk)) )
    {
        return;
    }

    mm_ResolveIt(lnk.toStdString().c_str(), app);
}

//retreive link from ProgramData instead of user account
void mm_getLinkPathAll(QString path, MmApplication *app)
{
    QString lnk = getenv("PROGRAMDATA");
    lnk += "\\Microsoft\\Windows\\Start Menu\\Programs\\";
    lnk += path;

    mm_ResolveIt(lnk.toStdString().c_str(), app);
}

HRESULT mm_ResolveIt(LPCSTR lnk_path, MmApplication *app)
{
    HRESULT hr;
    IShellLink* psl;
    WCHAR szTargetPath[MAX_PATH];
    WCHAR szDirPath[MAX_PATH];
    WCHAR szDescription[MAX_PATH];
    WIN32_FIND_DATA wfd;

    app->exe_path = ""; // Assume failure

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID*)&psl);
    if( hr )
    {
        qDebug() << "IID_IShellLink Failed" << hr;
        return hr;
    }

    IPersistFile* ppf;

    // Get a pointer to the IPersistFile interface.
    hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if( hr )
    {
        qDebug() << "IID_IPersistFile Failed" << hr;
        return hr;
    }

    WCHAR wsz[MAX_PATH];

    // Ensure that the string is Unicode.
    MultiByteToWideChar(CP_ACP, 0, lnk_path, -1, wsz, MAX_PATH);

    // Add code here to check return value from MultiByteWideChar
    // for success.

    // Load the shortcut.
    hr = ppf->Load(wsz, STGM_READ);
    if( hr )
    {
        qDebug() << "ppf->Load Failed" << hr;
        return hr;
    }

    // Resolve the link.
    HWND hwnd = GetActiveWindow();
    hr = psl->Resolve(hwnd, 0);

    if( SUCCEEDED(hr) )
    {
        // Get the path to the link target.
        hr = psl->GetPath(szTargetPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);
        hr = psl->GetWorkingDirectory(szDirPath, MAX_PATH);

        // Get the description of the target.
        hr = psl->GetDescription(szDescription, MAX_PATH);

        app->exe_path    = QString::fromStdWString(szTargetPath);
        app->working_dir = QString::fromStdWString(szDirPath);
    }

    // Release the pointer to the IPersistFile interface.
    ppf->Release();

    // Release the pointer to the IShellLink interface.
    psl->Release();

    return hr;
}

void mm_launchLnk(QString app_name, QString arg)
{
    app_name += ".lnk";
    MmApplication app;
    mm_getLinkPath(app_name, &app);

    mm_launchApp(&app, arg);
}

void mm_launchApp(MmApplication *app, QString arg)
{
    if( arg.length() )
    {
        app->exe_path += " " + arg;
    }

    PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
    STARTUPINFOA StartupInfo; //This is an [in] parameter

    ZeroMemory( &StartupInfo, sizeof(StartupInfo) );
    StartupInfo.cb = sizeof(StartupInfo);
    ZeroMemory( &ProcessInfo, sizeof(ProcessInfo) );

    char app_cmd[200];
    strcpy(app_cmd, app->exe_path.toStdString().c_str());

    int ret = CreateProcessA(NULL, app_cmd, NULL,
                             NULL, FALSE, 0, NULL,
                             app->working_dir.toStdString().c_str(),
                             &StartupInfo, &ProcessInfo);
    if( ret==0 )
    {
        long last_error = GetLastError();
        qDebug() << "Error 26: CreateProcess failed" << last_error
                 << "path" << app->exe_path
                 << "dir" << app->working_dir;
    }
}

void mm_launchScript(QString path, QString arg)
{
    QString cmd = path + arg;

    PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
    STARTUPINFOA StartupInfo; //This is an [in] parameter

    ZeroMemory( &StartupInfo, sizeof(StartupInfo) );
    StartupInfo.cb = sizeof(StartupInfo);
    ZeroMemory( &ProcessInfo, sizeof(ProcessInfo) );

    char app_cmd[200];
    strcpy(app_cmd, path.toStdString().c_str());

    int ret = CreateProcessA(NULL, app_cmd, NULL,
                             NULL, FALSE, 0, NULL,
                             NULL, &StartupInfo,
                             &ProcessInfo);
    if( ret==0 )
    {
        long last_error = GetLastError();
        qDebug() << "Error 25: CreateProcess failed" << last_error
                 << "path" << path;
    }
}

void mm_closeWindow()
{
    DWORD dwCurrentThread = GetCurrentThreadId();
    DWORD dwFGThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    AttachThreadInput(dwCurrentThread, dwFGThread, TRUE);

    HWND hwnd = GetForegroundWindow();
    PostMessage(hwnd, WM_CLOSE, 0, 0);

    AttachThreadInput(dwCurrentThread, dwFGThread, FALSE);
}

void mm_focusWindow(HWND hwnd)
{
    DWORD dwCurrentThread = GetCurrentThreadId();
    DWORD dwFGThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    AttachThreadInput(dwCurrentThread, dwFGThread, TRUE);

    AllowSetForegroundWindow(ASFW_ANY);
    LockSetForegroundWindow(LSFW_UNLOCK);
    BringWindowToTop(hwnd);
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);

    // If window is minimzed
    if( IsIconic(hwnd) )
    {
        ShowWindow(hwnd, SW_RESTORE);
    }

    AttachThreadInput(dwCurrentThread, dwFGThread, FALSE);
}

// Function to get the window opacity
int  mm_getWindowOpacity(HWND hwnd)
{
    BYTE opacity = 0;
    DWORD flags = 0;

    if (GetLayeredWindowAttributes(hwnd, NULL, &opacity, &flags))
    {
        if (flags & LWA_ALPHA)
        {
            return opacity;
        }
    }

    return 255;  // Default opacity if unable to retrieve
}

int  aj_isProcOpen(QString name)
{
    DWORD pid_list[1024], byte_len;

    if( !EnumProcesses(pid_list, sizeof(pid_list), &byte_len) )
    {
        return 0;
    }

    int len = byte_len / sizeof(DWORD);
    for( int i=0 ; i<len ; i++ )
    {
        if( pid_list[i]!=0 )
        {
            QString pname = mm_getPName(pid_list[i]);
            if( pname.toLower()==name.toLower() )
            {
                qDebug() << "aj_isProcOpen"
                         << pname << pid_list[i];
                return 1;
            }
        }
    }

    return 0;
}

void aj_waitOpen(QString p_name)
{
    while( 1 )
    {
        QThread::msleep(10);
        if( aj_isProcOpen(p_name) )
        {
            break;
        }
    }
}
