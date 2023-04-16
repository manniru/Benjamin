#include "ab_console.h"
#include <QDebug>
#include <QDir>
#include <QThread>

AbConsole::AbConsole(QObject *parent) : QObject(parent)
{
    is_ready = 0;
}

AbConsole::~AbConsole()
{
    if ( !CloseHandle(h_in_write) )
    {
//        qDebug() << "StdInWr CloseHandle failed";
    }
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(h_in_read);
}

void AbConsole::startConsole(QString wsl_path)
{
    QString current_dir = QDir::currentPath();
    QDir::setCurrent(wsl_path);

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // stdout
    if( !CreatePipe(&handle, &handle_err, &saAttr, 0) )
    {
       qDebug() << "StdoutRd CreatePipe failed";
    }
    if( !SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0) )
    {
       qDebug() << "Stdout SetHandleInformation failed";
    }

    // stdin
    if( !CreatePipe(&h_in_read, &h_in_write, &saAttr, 0) )
    {
       qDebug() << "Stdin CreatePipe failed";
    }
    if( !SetHandleInformation(h_in_write, HANDLE_FLAG_INHERIT, 0) )
    {
       qDebug() << "Stdin SetHandleInformation failed";
    }

    CreateCmdProcess();
    QDir::setCurrent(current_dir);

    readData();
}

void AbConsole::readData()
{
    DWORD read_len;
    char chBuf[CONSOLE_BUF_SIZE];
    BOOL ok;
    QString output;

    while( true )
    {
        ok = ReadFile(handle, chBuf,
                      CONSOLE_BUF_SIZE, &read_len, NULL);
        if( !ok || read_len==0 )
        {
            qDebug() << "ERROR" << ok << read_len;
            break;
        }
        chBuf[read_len] = 0;

        output = chBuf;
        emit readyData(output, 0);
        qDebug() << output;
        processLine(output);
    }
}

void AbConsole::CreateCmdProcess()
{
   BOOL bSuccess = FALSE;

   ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

   STARTUPINFOA siStartInfo;
   ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
   siStartInfo.cb = sizeof(STARTUPINFO);
   siStartInfo.hStdError = handle_err;
   siStartInfo.hStdOutput = handle_err;
   siStartInfo.hStdInput = h_in_read;
   siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

   char cmd_a[100] = "cmd.exe";
   bSuccess = CreateProcessA(NULL,
      cmd_a,         // command line
      NULL,          // process security attributes
      NULL,          // primary thread security attributes
      TRUE,          // handles are inherited
      0,             // creation flags
      NULL,          // use parent's environment
      NULL,          // use parent's current directory
      &siStartInfo,  // STARTUPINFO pointer
      &piProcInfo);  // receives PROCESS_INFORMATION

   if ( !bSuccess )
   {
       qDebug() << "CreateProcess failed";
   }
}

void AbConsole::processLine(QString line)
{
    if( line.contains("Arch>") )
    {
        if( commands.length() )
        {
            QString cmd = commands.first();
            commands.removeFirst();
            DWORD dwWritten;
            WriteFile(h_in_write, cmd.toStdString().c_str(),
                      cmd.length(), &dwWritten, NULL);
        }
        else
        {
            is_ready = 1;
            emit finished();
        }
    }
}

void AbConsole::run(QString cmd)
{
    DWORD dwWritten;

    QString wsl_cmd = "KalB.exe run ";
    wsl_cmd += cmd + "\n";

    if( is_ready )
    {
        qDebug() << "Write" << cmd;
        WriteFile(h_in_write, wsl_cmd.toStdString().c_str(),
                  wsl_cmd.length(), &dwWritten, NULL);
        is_ready = 0;
    }
    else
    {
        qDebug() << "Busy" << cmd;
        commands << wsl_cmd;
    }
}
