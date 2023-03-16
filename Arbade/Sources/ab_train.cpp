#include "ab_train.h"
#include <QDebug>

AbTrain::AbTrain(QObject *ui, QObject *parent) : QObject(parent)
{
    wsl = new AbInitWSL();
    root = ui;
    wsl_dialog = root->findChild<QObject*>("WslDialog");

    connect(root, SIGNAL(sendKey(int)), this, SLOT(processKey(int)));
    connect(wsl_dialog, SIGNAL(driveEntered(QString)),
            wsl, SLOT(createWSL(QString)));
}

AbTrain::~AbTrain()
{

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
    QString path = wsl->getWslPath();
    if( path.isEmpty() )
    {
        QMetaObject::invokeMethod(root, "initWsl");
    }

    if( !QFile::exists(path + "\\arch.exe") )
    {
        QString drive = QString(path[0]);
        wsl->createWSL(drive);
    }
}
