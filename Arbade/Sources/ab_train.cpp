#include "ab_train.h"
#include <QDebug>
#include <QQmlProperty>

AbTrain::AbTrain(QObject *ui, QObject *parent) : QObject(parent)
{
    wsl = new AbInitWSL();
    wsl_thread = new QThread();
    wsl->moveToThread(wsl_thread);

    wsl_thread->start();
    root = ui;
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
    connect(console, SIGNAL(finished()), this, SLOT(trainFinished()));

    connect(this, SIGNAL(createWSL(QString)),
            wsl, SLOT(createWSL(QString)));
    connect(wsl, SIGNAL(WslCreated()),
            this, SLOT(WslCreated()));

    enn_console = new AbConsole(AB_CONSOLE_NORML);
    enn_thread = new QThread();
    enn_console->moveToThread(enn_thread);
    enn_thread->start();
    connect(enn_console, SIGNAL(readyData(QString,int)),
            this, SLOT(writeToQml(QString,int)));
    connect(this, SIGNAL(startEnnConsole(QString)),
            enn_console, SLOT(startConsole(QString)));

    QString current_dir = QDir::currentPath();
    emit startEnnConsole(current_dir);

    initWsl();
}

AbTrain::~AbTrain()
{
}

void AbTrain::processKey(int key)
{
    qDebug() << "k" << key;
    if( key==Qt::Key_T )
    {
        train();
    }
    else if( key==Qt::Key_Escape )
    {
        ;
    }
    else if( key==Qt::Key_D )
    {
        createENN();
    }
    else if( key==Qt::Key_B )
    {

    }
}

void AbTrain::WslCreated()
{
    emit startConsole(wsl_path);
}

void AbTrain::trainFinished()
{
    qDebug() << QDir::currentPath();
    checkModelExist();

    QVector<QString> files;

    files << "HCLG.fst";
    files << "words.txt";
    files << "final.oalimdl";
    files << "global_cmvn.stats";

    int len = files.length();
    for( int i=0 ; i<len ; i++ )
    {
        QString old_path = wsl_path + "\\Benjamin\\Tools\\Model\\";
        old_path += files[i];
        QString new_path = AB_MODEL_DIR;
        new_path +=  "\\" + files[i];
        QFile file(old_path);
        file.copy(new_path);
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
        emit createWSL(drive);
    }
    else
    {
        emit startConsole(wsl_path);
    }
}

void AbTrain::train()
{
    qDebug() << "train KalB";
    QQmlProperty::write(console_qml, "visible", true);
    console->wsl_run("./wsl_init.sh");
    console->wsl_run("./wsl_train.sh");
}

void AbTrain::createENN()
{
    qDebug() << "createENN";
    QQmlProperty::write(console_qml, "visible", true);
    enn_console->run("dir ..\\Tools\\release\\");
    enn_console->run("..\\Tools\\release\\BaTool.exe e 2>&1");
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

void AbTrain::checkModelExist()
{
    QString model_path = AB_MODEL_DIR;
    QDir model_dir(model_path);

    if( !model_dir.exists() )
    {
        qDebug() << "Creating" << model_path
                 << " Directory";
#ifdef WIN32
        QString cmd = "mkdir " + model_path;
        system(cmd.toStdString().c_str());
#else //OR __linux
        system("mkdir -p ../Tools/Model");
#endif
    }
}
