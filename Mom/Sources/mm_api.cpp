#include "mm_api.h"
#include <QDebug>
#include <shobjidl.h>
#include <shlguid.h>

QString mm_getLinkPath(QString path)
{
    QString ret = mm_getLinkPathA(path);
    if( ret.isEmpty() )
    {
        ret = mm_getLinkPathB(path);
    }
    return ret;
}

QString mm_getLinkPathA(QString path)
{
    char target[MAX_PATH];

    QString lnk = getenv("APPDATA");
    lnk += "\\Microsoft\\Windows\\Start Menu\\Programs\\";
    lnk += path;

    mm_ResolveIt(lnk.toStdString().c_str(), target);

    return target;
}

//retreive link from ProgramData instead of user account
QString mm_getLinkPathB(QString path)
{
    char target[MAX_PATH];

    QString lnk = getenv("PROGRAMDATA");
    lnk += "\\Microsoft\\Windows\\Start Menu\\Programs\\";
    lnk += path;

    mm_ResolveIt(lnk.toStdString().c_str(), target);

    return target;
}

HRESULT mm_ResolveIt(LPCSTR lnk_path, char *target)
{
    HRESULT hres;
    IShellLink* psl;
    WCHAR szGotPath[MAX_PATH];
    WCHAR szDescription[MAX_PATH];
    WIN32_FIND_DATA wfd;

    *target = 0; // Assume failure

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Get a pointer to the IPersistFile interface.
        hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

        if (SUCCEEDED(hres))
        {
            WCHAR wsz[MAX_PATH];

            // Ensure that the string is Unicode.
            MultiByteToWideChar(CP_ACP, 0, lnk_path, -1, wsz, MAX_PATH);

            // Add code here to check return value from MultiByteWideChar
            // for success.

            // Load the shortcut.
            hres = ppf->Load(wsz, STGM_READ);

            if (SUCCEEDED(hres))
            {
                // Resolve the link.
                HWND hwnd = GetActiveWindow();
                hres = psl->Resolve(hwnd, 0);

                if (SUCCEEDED(hres))
                {
                    // Get the path to the link target.
                    hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);

                    if (SUCCEEDED(hres))
                    {
                        // Get the description of the target.
                        hres = psl->GetDescription(szDescription, MAX_PATH);

                        if (SUCCEEDED(hres))
                        {
                            hres = wcstombs(target, szGotPath, wcslen(szGotPath) + 1);
                            if (SUCCEEDED(hres))
                            {
                                // Handle success
                            }
                            else
                            {
                                // Handle the error
                            }
                        }
                    }
                }
            }

            // Release the pointer to the IPersistFile interface.
            ppf->Release();
        }

        // Release the pointer to the IShellLink interface.
        psl->Release();
    }
    return hres;
}

void mm_launchApp(QString app_name, QString arg)
{
    app_name += ".lnk";
    QString path = mm_getLinkPath(app_name);
    path += " " + arg;

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
    if( ret == 0 )
    {
        long last_error = GetLastError();
        qDebug() << "CreateProcess failed" << last_error;
    }
}
