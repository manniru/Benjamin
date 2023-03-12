#include "ab_init_wsl.h"
#include <QDebug>

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
            if( dir_list[j].fileName()=="Arch" )
            {
                return dir_list[j].filePath();
            }
        }

    }
    return "";
}
