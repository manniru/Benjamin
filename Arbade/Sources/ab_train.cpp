#include "ab_train.h"
#include <QDebug>

AbTrain::AbTrain(QObject *ui, QObject *parent) : QObject(parent)
{
    wsl = new AbInitWSL();
    root = ui;
    connect(root, SIGNAL(sendKey(int)), this, SLOT(processKey(int)));
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
}
