#include "ab_train.h"
#include <QDebug>
#include <QQmlProperty>

AbTrain::AbTrain(QObject *ui, QObject *parent) : QObject(parent)
{
    wsl = new AbInitWSL();
    root = ui;
    init_flag = 1;
    wsl_dialog = root->findChild<QObject*>("WslDialog");
    console_qml = root->findChild<QObject*>("Console");
    connect(root, SIGNAL(sendKey(int)), this, SLOT(processKey(int)));
    connect(wsl_dialog, SIGNAL(driveEntered(QString)),
            wsl, SLOT(createWSL(QString)));

    console = new AbConsole(AB_CONSOLE_NORML);
    con_thread = new QThread();
    console->moveToThread(con_thread);
    con_thread->start();
    connect(console, SIGNAL(readyData(QString,int)),
            this, SLOT(writeToQml(QString,int)));
    connect(this, SIGNAL(startConsole(QString)),
            console, SLOT(startConsole(QString)));
}

AbTrain::~AbTrain()
{
}

void AbTrain::processKey(int key)
{
    if( key==Qt::Key_T )
    {
        if( init_flag )
        {
            initWsl();
        }
        initKalB();
    }
    else if( key==Qt::Key_Escape )
    {
        ;
    }
    else if( key==Qt::Key_B )
    {

    }
    else
    {
        return;
    }
}

void AbTrain::initWsl()
{
    wsl_path = ab_getWslPath();
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
    emit startConsole(wsl_path);

    init_flag = 0;
}

void AbTrain::initKalB()
{
    qDebug() << "createKalB";
    QQmlProperty::write(console_qml, "visible", true);
    console->run("./wsl_init.sh");
    console->run("./wsl_train.sh");
}

void AbTrain::writeToQml(QString line, int flag)
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
        QQmlProperty::write(console_qml, "line_buf", line_fmt);
//        qDebug() << i << "line_fmt" << line_fmt;
        QMetaObject::invokeMethod(console_qml, "addLine");
    }
}
