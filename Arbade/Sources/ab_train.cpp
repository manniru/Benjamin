#include "ab_train.h"
#include <QDebug>
#include <QQmlProperty>

AbTrain::AbTrain(QObject *ui, QObject *parent) : QObject(parent)
{
    wsl = new AbInitWSL();
    root = ui;
    init_flag = 1;
    wsl_dialog = root->findChild<QObject*>("WslDialog");
    console = root->findChild<QObject*>("Console");
    connect(root, SIGNAL(sendKey(int)), this, SLOT(processKey(int)));
    connect(wsl_dialog, SIGNAL(driveEntered(QString)),
            wsl, SLOT(createWSL(QString)));

    con_read = new AbConsoleController(AB_CONSOLE_NORML);
    con_thread = new QThread();
    con_read->moveToThread(con_thread);
    con_thread->start();
    connect(con_read, SIGNAL(readyData(QString, int)),
            this, SLOT(writeConsole(QString, int)));
    connect(this, SIGNAL(readConsole()),
            con_read, SLOT(run()));
    connect(con_read, SIGNAL(sendCommand(QString)),
            this, SLOT(WriteToPipe(QString)));

    err_read = new AbConsoleController(AB_CONSOLE_ERROR);
    err_thread = new QThread();
    err_read->moveToThread(err_thread);
    err_thread->start();
    connect(err_read, SIGNAL(readyData(QString, int)),
            this, SLOT(writeConsole(QString, int)));
    connect(this, SIGNAL(readError()),
            err_read, SLOT(run()));
    connect(err_read, SIGNAL(sendCommand(QString)),
            this, SLOT(WriteToPipe(QString)));
}

AbTrain::~AbTrain()
{
    qDebug() << "Closing handles";
    if ( !CloseHandle(h_in_write) )
    {
        qDebug() << "StdInWr CloseHandle failed";
    }
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(err_read->handle);
    CloseHandle(h_in_read);
}

void AbTrain::processKey(int key)
{
    if( key==Qt::Key_T )
    {
        if( init_flag )
        {
            initWsl();
        }
        else
        {
            initKalB();
        }
    }
    else if( key==Qt::Key_Escape )
    {
        ;
    }
    else
    {
        return;
    }
}

void AbTrain::initWsl()
{
    QString wsl_path = ab_getWslPath();
    if( wsl_path.isEmpty() )
    {
        qDebug() << "Error 152: shit has happened";
        exit(52);
    }
    if( !QFile::exists(wsl_path + "\\ext4.vhdx") )
    {
        QString drive = QString(wsl_path[0]);
        wsl->createWSL(drive);
    }

    qDebug() << "createKalB";
    initKalB();
}

void AbTrain::initKalB()
{
    QQmlProperty::write(root, "ab_show_console", 1);
    QString current_dir = QDir::currentPath();
    QDir::setCurrent(wsl_path);

    openConsole();
}

void AbTrain::writeConsole(QString line, int flag)
{
    QString color = "#ffffff";
    QStringList lines = line.split("\n");
    int count = lines.count();
    for( int i=0; i<count ; i++)
    {
        QString line_fmt;
        line_fmt += lines[i];
        if( flag==AB_CONSOLE_ERROR )
        {
            QString line_fmt = "<font style=\"color: ";
            color = "#00f";
            line_fmt += color;
            line_fmt += ";\">";
            line_fmt += line;
            line_fmt += "</font>";
        }

        if( i<count-1 )
        {
            line_fmt += "<br>";
        }
        QQmlProperty::write(console, "line_buf", line_fmt);
//        qDebug() << i << "line_fmt" << line_fmt;
        QMetaObject::invokeMethod(console, "addLine");
    }
}

int AbTrain::openConsole()
{
    qDebug() << "openConsole";
    SECURITY_ATTRIBUTES saAttr;
    if( init_flag )
    {
        con_read->line_number = 0;
        err_read->line_number = 0;
    }
    else
    {
        con_read->line_number = 1;
        err_read->line_number = 1;
    }
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // stdout
    if( !CreatePipe(&con_read->handle, &err_read->handle, &saAttr, 0) )
    {
       qDebug() << "StdoutRd CreatePipe failed";
    }
    if( !SetHandleInformation(con_read->handle, HANDLE_FLAG_INHERIT, 0) )
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

    CreateChildProcess("cmd.exe");

    emit readConsole();
    emit readError();

    init_flag = 0;

    return 0;
}

void AbTrain::CreateChildProcess(QString cmd)
{
   BOOL bSuccess = FALSE;

   ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

   STARTUPINFOA siStartInfo;
   ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
   siStartInfo.cb = sizeof(STARTUPINFO);
   siStartInfo.hStdError = err_read->handle;
   siStartInfo.hStdOutput = err_read->handle;
   siStartInfo.hStdInput = h_in_read;
   siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

   char cmd_a[100];
   strcpy(cmd_a, cmd.toStdString().c_str());
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

void AbTrain::WriteToPipe(QString cmd)
{
    DWORD dwWritten;
    qDebug() << "Write" << cmd;

    WriteFile(h_in_write, cmd.toStdString().c_str(),
              cmd.length(), &dwWritten, NULL);

}
