#include "ab_init_wsl.h"
#include <QDebug>

AbInitWSL::AbInitWSL(QObject *parent) : QObject(parent)
{

}

AbInitWSL::~AbInitWSL()
{

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

    QString rar_path = arch_dir + "\\";
    rar_path += AB_IMAGE_FILENAME;
    QString tar_path = arch_dir + "\\install.tar";
    if( !QFile::exists(rar_path) && !QFile::exists(tar_path) )
    {
        qDebug() << "Info: Downloading" << AB_IMAGE_URL;
        downloadImage(rar_path);
    }

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
        qDebug() << "Info: Unregister Any Previous Version";
        system("wsl --unregister KalB");
        qDebug() << "Info: Installing KalB Virtual Machine";
        system("KalB.exe install install.tar");
    }
    QDir::setCurrent(current_dir);

    emit WslCreated();
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
