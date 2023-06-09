#include "ab_console.h"
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QQmlProperty>

AbConsole::AbConsole(QObject *ui, QObject *parent) : QObject(parent)
{
    is_ready = 1;
    init_shit = 1;
    root = ui;
    console_qml = root->findChild<QObject*>("Console");

    std_out = new AbConsoleHandle(AB_CONSOLE_NORML);
    out_thread = new QThread;
    std_out->moveToThread(out_thread);
    out_thread->start();
    std_err = new AbConsoleHandle(AB_CONSOLE_ERROR);
    err_thread = new QThread;
    std_err->moveToThread(err_thread);
    err_thread->start();

    connect(this, SIGNAL(startRead()),
            std_err, SLOT(readData()));
    connect(std_err, SIGNAL(readyData(QString,int)),
            this, SLOT(readyData(QString,int)));
    connect(this, SIGNAL(startRead()),
            std_out, SLOT(readData()));
    connect(std_out, SIGNAL(readyData(QString,int)),
            this, SLOT(readyData(QString,int)));
}

AbConsole::~AbConsole()
{
    qDebug() << "Terminating Process";
    TerminateProcess(piProcInfo.hProcess, 255);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(proc_in_h);
    CloseHandle(handle_in);
}

void AbConsole::startConsole(QString wsl_path)
{
    QString current_dir = QDir::currentPath();
    QDir::setCurrent(wsl_path);
    QDir dir_info(wsl_path);
    prompt = dir_info.dirName() + ">";

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // stdout
    if( !CreatePipe(&(std_out->handle), &proc_out_h, &saAttr, 0) )
    {
       qDebug() << "StdoutRd CreatePipe failed";
    }
    if( !SetHandleInformation(std_out->handle, HANDLE_FLAG_INHERIT, 0) )
    {
       qDebug() << "Stdout SetHandleInformation failed";
    }

    // stdin
    if( !CreatePipe(&proc_in_h, &handle_in, &saAttr, 0) )
    {
       qDebug() << "Stdin CreatePipe failed";
    }
    if( !SetHandleInformation(handle_in, HANDLE_FLAG_INHERIT, 0) )
    {
       qDebug() << "Stdin SetHandleInformation failed";
    }

    // stderr
    if( !CreatePipe(&std_err->handle, &proc_err_h, &saAttr, 0) )
    {
       qDebug() << "Stderr CreatePipe failed";
    }
    if( !SetHandleInformation(std_err->handle, HANDLE_FLAG_INHERIT, 0) )
    {
       qDebug() << "Stderr SetHandleInformation failed";
    }

    CreateCmdProcess();
    QDir::setCurrent(current_dir);

    qDebug() << "console" << current_dir;
    emit startRead();
}

void AbConsole::CreateCmdProcess()
{
   BOOL ret;
   ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

   STARTUPINFO siStartInfo;
   ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
   siStartInfo.cb = sizeof(STARTUPINFO);
   siStartInfo.hStdError  = proc_err_h;
   siStartInfo.hStdOutput = proc_out_h;
   siStartInfo.hStdInput  = proc_in_h;
   // STARTF_USESHOWWINDOW prevent from showing cmd window
   siStartInfo.dwFlags   |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

   // CREATE_NEW_CONSOLE prevent weired no output on some application
   wchar_t cmd_a[100] = L"cmd.exe";
   ret = CreateProcess(NULL,
      cmd_a,              // command line
      NULL,               // process security attributes
      NULL,               // primary thread security attributes
      TRUE,               // handles are inherited
      CREATE_NEW_CONSOLE, // creation flags
      NULL,               // use parent's environment
      NULL,               // use parent's current directory
      &siStartInfo,       // STARTUPINFO pointer
      &piProcInfo);       // receives PROCESS_INFORMATION

   if ( ret==0 )
   {
       qDebug() << "CreateProcess failed";
   }
}

void AbConsole::processLine(QString line)
{
    if( line.contains(prompt) )
    {
        if( commands.length() )
        {
            QString cmd = commands.first();
            commands.removeFirst();
            DWORD dwWritten;
            WriteFile(handle_in, cmd.toStdString().c_str(),
                      cmd.length(), &dwWritten, NULL);
        }
        else
        {
            is_ready = 1;
            emit finished();
        }
    }
    if( line.contains("error", Qt::CaseInsensitive) )
    {
        emit trainFailed();
    }
}

void AbConsole::runAll()
{
    run(commands[0]);
}

void AbConsole::wsl_run(QString cmd)
{
    QString wsl_cmd = "KalB.exe run " + cmd;
    run(wsl_cmd);
}

void AbConsole::run(QString cmd)
{
    DWORD dwWritten;
    cmd += "\r\n";
    init_shit = 0;

    if( is_ready )
    {
        qDebug() << "Write" << cmd;
        WriteFile(handle_in, cmd.toStdString().c_str(),
                  cmd.length(), &dwWritten, NULL);
        is_ready = 0;
    }
    else
    {
        qDebug() << "Busy" << cmd;
        commands << cmd;
    }
}

void AbConsole::readyData(QString line, int flag)
{
    if( init_shit )
    {
        return;
    }
    processLine(line);

    QString color = "#ccc";
    QStringList lines = line.split("\n");
    int count = lines.count();
    int new_line = 0;
    if( line.back()=="\n" )
    {
        new_line = 1;
    }

    for( int i=0; i<count ; i++)
    {
        if( lines[i]=="\r" || lines[i].isEmpty() )
        {
            continue;
        }

        if( i<count-1 || new_line )
        {
            QQmlProperty::write(console_qml, "line_buf", lines[i]+"\n");
            QMetaObject::invokeMethod(console_qml, "addLine");
        }
        else
        {
            QQmlProperty::write(console_qml, "line_buf", lines[i]);
            QMetaObject::invokeMethod(console_qml, "addText");
        }
        qDebug() << i << "line_fmt" << lines[i] << line;
    }
}
