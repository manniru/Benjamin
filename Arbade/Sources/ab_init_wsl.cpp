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
        qDebug() << "Info: Directory" << arch_dir << "created";
    }

    QString rar_path = arch_dir + "\\KalB.rar";
    if( !QFile::exists(rar_path) )
    {
        qDebug() << "Info: Downloading" << AB_IMAGE_URL;
        downloadImage(rar_path);
    }

    QString tar_path = arch_dir + "\\KalB.tar";
    if( !QFile::exists(tar_path) )
    {
        qDebug() << "Info: Uncompressing" << tar_path;
        uncompImage(rar_path);
    }

    if( checkWSLInstalled()==0 )
    {
        qDebug() << "Info: Installing WSL";
        system("start cmd /c wsl --install");
    }
}

void AbInitWSL::downloadImage(QString path)
{
    QString cmd = "curl -L " AB_IMAGE_URL;
    cmd += " --output " + path;
    system(cmd.toStdString().c_str());
}

    }
    return "";
}
