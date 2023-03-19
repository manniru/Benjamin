#include "ab_train.h"
#include <QDebug>
#include <QQmlProperty>

AbTrain::AbTrain(QObject *ui, QObject *parent) : QObject(parent)
{
    wsl = new AbInitWSL();
    root = ui;
    wsl_dialog = root->findChild<QObject*>("WslDialog");
    console = root->findChild<QObject*>("Console");
    connect(root, SIGNAL(sendKey(int)), this, SLOT(processKey(int)));
    connect(wsl_dialog, SIGNAL(driveEntered(QString)),
            wsl, SLOT(createWSL(QString)));

    con_read = new QProcess(parent);
    connect(con_read, SIGNAL(readyRead()),
            this, SLOT(writeConsole()));
    connect(con_read, SIGNAL(readyReadStandardError()),
            this, SLOT(writeEConsole()));
}

AbTrain::~AbTrain()
{
    if ( !CloseHandle(h_in_write) )
    {
        qDebug() << "StdInWr CloseHandle failed";
    }
}

void AbTrain::processKey(int key)
{
    if( key==Qt::Key_T )
    {
        initWsl();
    }
    else
    {
        return;
    }
}

void AbTrain::initWsl()
{
    qDebug() << "TRAINING STARTED ....";
    wsl_path = wsl->getWslPath();
    if( wsl_path.isEmpty() )
    {
        QMetaObject::invokeMethod(root, "initWsl");
    }

    if( !QFile::exists(wsl_path + "\\ext4.vhdx") )
    {
        QString drive = QString(wsl_path[0]);
        wsl->createWSL(drive);
    }

    qDebug() << "createKalB";
    createKalB();
}

void AbTrain::createKalB()
{
    QQmlProperty::write(root, "ab_show_console", 1);
    QString current_dir = QDir::currentPath();
    QDir::setCurrent(wsl_path);

    openApp();
}

void AbTrain::writeConsole()
{
    QString line = con_read->readAll();
    QString color = "#ffffff";
    int ll_flag = 0; //last line
    if( line[line.length()-1]=='\n' )
    {
        ll_flag = 1;
    }
    QStringList lines = line.split("\n");
    int count = lines.count();
    for( int i=0; i<count ; i++)
    {
        QString line_fmt;
        line_fmt += lines[i];

        if( i<count-1 )
        {
            line_fmt += "<br>";
        }
        else if( ll_flag )
        {
            line_fmt += "<br>";
        }
        QQmlProperty::write(console, "line_buf", line_fmt);
        qDebug() << "line_fmt" << line_fmt;
        QMetaObject::invokeMethod(console, "addLine");
        processLine(lines[i]);
    }
}


void AbTrain::writeEConsole()
{
    QString line = con_read->readAllStandardError();
    QString color = "#ff0000";
    int ll_flag = 0; //last line
    if( line[line.length()-1]=='\n' )
    {
        ll_flag = 1;
    }
    QStringList lines = line.split("\n");
    int count = lines.count();
    for( int i=0; i<count ; i++)
    {
        QString line_fmt;
        line_fmt += "<font style=\"color: ";
        line_fmt += color;
        line_fmt += ";\">";
        line_fmt += lines[i];
        line_fmt += "</font>";

        if( i<count-1 )
        {
            line_fmt += "<br>";
        }
        else if( ll_flag )
        {
            line_fmt += "<br>";
        }
        QQmlProperty::write(console, "line_buf", line_fmt);
        qDebug() << "line_fmt" << line_fmt.toStdString().c_str();
        QMetaObject::invokeMethod(console, "addLine");
        processLine(lines[i]);
    }
}

void AbTrain::processLine(QString line)
{
    if( line.contains("Arch>") )
    {
        if( state==0 )
        {
            QString cmd = "KalbnB.exe\n";
//            QString cmd = "dir\n";
        //    QString cmd = "ls\n";
            state = 1;
            con_read->write(cmd.toStdString().c_str());
            con_read->waitForBytesWritten();
        }
    }
}


int AbTrain::openApp()
{
//    CreateChildProcess("KalB.exe");
//    con_read->start("KalB.exe");
    con_read->start("cmd.exe");

    return 0;
}

void AbTrain::WriteToPipe(QString cmd)
{
    DWORD dwWritten;
    qDebug() << "Write" << cmd;

    WriteFile(h_in_write, cmd.toStdString().c_str(),
              cmd.length(), &dwWritten, NULL);

}
