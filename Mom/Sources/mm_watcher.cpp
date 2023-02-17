#include "mm_watcher.h"
#include <QFileInfo>

MmWatcher::MmWatcher(QObject *parent) : QObject(parent)
{   
    appNames << "Chess.exe";   app_h << NULL;
    appNames << "Rebound.exe"; app_h << NULL;
    appNames << "BaTool.exe";  app_h << NULL;

    // Timer
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start(MM_WATCHER_TIMER);
}

bool MmWatcher::isAppRunning(QString name)
{
    bool exists = false;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if(Process32First(snapshot, &entry))
    {
        while (Process32Next(snapshot, &entry))
        {
            QString p_name = QString::fromStdWString(entry.szExeFile);
            if( p_name==name )
            {
                exists = true;
                break;
            }
        }
    }

    CloseHandle(snapshot);
    return exists;
}

void MmWatcher::timeout()
{
    int length = appNames.length();
    for( int i=0 ; i<length ; i++ )
    {
        int is_running = isAppRunning(appNames[i]);
        if( is_running==0 )
        {
            if( appNames[i]=="Chess.exe" )
            {
                app_h[i] = run(MM_CHESS_PATH);
            }
            else if( appNames[i]=="BaTool.exe" )
            {
                app_h[i] = run(MM_BATOOL_PATH);
            }
            else if( appNames[i]=="Rebound.exe" )
            {
                app_h[i] = run(MM_REBOUND_PATH);
            }
        }
    }
}

HANDLE MmWatcher::run(QString path)
{
    qDebug() << "run" << path;
    PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
    STARTUPINFOA StartupInfo; //This is an [in] parameter

    ZeroMemory( &StartupInfo, sizeof(StartupInfo) );
    StartupInfo.cb = sizeof(StartupInfo);
    ZeroMemory( &ProcessInfo, sizeof(ProcessInfo) );

    QFileInfo file_info(path);
    QString working_dir = file_info.absolutePath().replace("/","\\");
    QString command = "\"" + path.replace("/","\\") + "\"";
    char app_cmd[200];
    char app_dir[200];
    strcpy(app_cmd, command.toStdString().c_str());
    strcpy(app_dir, working_dir.toStdString().c_str());

    CreateProcessA(NULL, app_cmd, NULL,
                   NULL, FALSE, 0, NULL,
                   app_dir, &StartupInfo,
                   &ProcessInfo);

    return ProcessInfo.hProcess;
}
