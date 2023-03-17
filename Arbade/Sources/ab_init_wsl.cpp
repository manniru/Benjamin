#include "ab_init_wsl.h"
#include <QDebug>
#include <shobjidl.h>
#include <shlguid.h>

AbInitWSL::AbInitWSL(QObject *parent) : QObject(parent)
{

}

AbInitWSL::~AbInitWSL()
{

}

QString AbInitWSL::getWslPath()
{
    QFileInfoList drives = QDir::drives();
    int len = drives.size();
    for( int i=0 ; i<len ; i++ )
    {
        QDir arch_dir(drives[i].filePath());
        QFileInfoList dir_list = arch_dir.entryInfoList(QDir::Dirs);
        int dirs_num = dir_list.size();
        for( int j=0 ; j<dirs_num ; j++ )
        {
            if( dir_list[j].fileName()==AB_WSL_ROOT )
            {
                return dir_list[j].filePath();
            }
        }
    }

    return "";
}

void AbInitWSL::createWSL(QString drive)
{
    QString arch_dir = drive.toUpper();
    arch_dir += ":\\" AB_WSL_ROOT;
    QDir dir_tester(arch_dir);
    if( !dir_tester.exists() )
    {
        QString cmd = "mkdir " + arch_dir;
        system(cmd.toStdString().c_str());
        qDebug() << "Info: Directory" << arch_dir << "Created";
    }

    QString rar_path = arch_dir + "\\KalB.rar";
    if( !QFile::exists(rar_path) )
    {
        qDebug() << "Info: Downloading" << AB_IMAGE_URL;
        downloadImage(rar_path);
    }

    QString tar_path = arch_dir + "\\install.tar";
    if( !QFile::exists(tar_path) )
    {
        qDebug() << "Info: Uncompressing" << tar_path;
        uncompImage(rar_path);
    }

    if( checkWSLInstalled()==0 )
    {
        qDebug() << "Info: Installing WSL";
        system("start cmd /c wsl --install");
//        system("shutdown /r");
        return;
    }

    QString vhd_path = arch_dir + "\\ext4.vhdx";
    QString current_dir = QDir::currentPath();
    QDir::setCurrent(arch_dir);
    if( !QFile::exists(vhd_path) )
    {
        qDebug() << "Info: Installing KalB Virtual Machine";
        system("KalB.exe");
    }
    QDir::setCurrent(current_dir);
}

void AbInitWSL::downloadImage(QString path)
{
    QString cmd = "curl -L " AB_IMAGE_URL;
    cmd += " --output " + path;
    system(cmd.toStdString().c_str());
}

void AbInitWSL::uncompImage(QString path)
{
    QString winrar_exe = getLinkPath("winrar");
    QString unrar_exe = winrar_exe.mid(0, winrar_exe.lastIndexOf('\\'));
    unrar_exe += "\\UnRAR.exe";
    if( !QFile::exists(unrar_exe) )
    {
        qDebug() << "Error: Uncompressing failed, UnRAR.exe not found";
    }
    QString cmd = "\"" + unrar_exe + "\" x " + path;
    cmd += " " + path.mid(0, path.lastIndexOf('\\'));
    system(cmd.toStdString().c_str());
}

int AbInitWSL::checkWSLInstalled()
{
//    QString output = getStrCommand("wsl -l");
    QString output = getStrCommand("echo hello");
    qDebug() << output;
    if( output.isEmpty() )
    {
        return 0;
    }
    return 1;
}

QString getLinkPath(QString name)
{
    QString ret = getLinkPathA(name);
    if( ret.isEmpty() )
    {
        ret = getLinkPathB(name);
    }
    return ret;
}

QString getLinkPathA(QString name)
{
    char target[300];
    QString black_list;

    while(true)
    {
        QString lnk = getenv("APPDATA");
        lnk += "\\Microsoft\\Windows\\Start Menu\\Programs\\";
        lnk = findAppPath(lnk, name, black_list);

        resolveIt(lnk.toStdString().c_str(), target);

        if( QString(target).contains(".exe") )
        {
            break;
        }
        black_list += lnk + "!";
    }

    return target;
}

//retreive link from ProgramData instead of user account
QString getLinkPathB(QString name)
{
    char target[300];

    QString lnk = getenv("PROGRAMDATA");
    lnk += "\\Microsoft\\Windows\\Start Menu\\Programs\\";
    lnk = findAppPath(lnk, name,"");

    resolveIt(lnk.toStdString().c_str(), target);

    return target;
}

QString findAppPath(QString path, QString pattern, QString black_list)
{
    QDir directory(path);
    directory.setFilter(QDir::Files | QDir::NoDot | QDir::NoDotDot);
    QRegExp pattern_reg("^" + pattern.toLower());
    QRegExp lnk_reg(".lnk$");

    if( directory.exists() )
    {
        QFileInfoList file_list = directory.entryInfoList();

        for( int i=0 ; i<file_list.size() ; i++ )
        {
            if( file_list[i].fileName().toLower().contains(pattern_reg) &&
                file_list[i].fileName().contains(lnk_reg))
            {
                QString ret = file_list[i].absoluteFilePath().replace("/", "\\");
                if( black_list.contains(ret) )
                {
                    continue;
                }
                return ret;
            }
        }
        directory.setFilter(QDir::Dirs | QDir::NoSymLinks |
                            QDir::NoDot | QDir::NoDotDot);

        QFileInfoList dir_list = directory.entryInfoList();

        for( int i=0 ; i<dir_list.size() ; i++ )
        {
            if( dir_list[i].fileName().toLower().contains(pattern_reg) )
            {
                return findAppPath(dir_list[i].absoluteFilePath()
                                   .replace("/", "\\"),  pattern, black_list);
            }
        }
        return "";
    }
    else
    {
        qDebug() << "Error: Directory doesnt exist.";
        return "";
    }
}

HRESULT resolveIt(LPCSTR lnk_path, char *target)
{
    HRESULT hres;
    IShellLink* psl;
    WCHAR szGotPath[MAX_PATH];
    WCHAR szDescription[MAX_PATH];
    WIN32_FIND_DATA wfd;

    *target = 0; // Assume failure

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID*)&psl);
    if( SUCCEEDED(hres) )
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
