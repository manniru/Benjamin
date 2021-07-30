#include "bt_online.h"

BtOnline::BtOnline(QObject *parent) : QObject(parent)
{
    record_thread = new QThread;
    encoder_thread = new QThread;

    recorder = new BtRecoder(record_thread);
    recorder->moveToThread(record_thread);
    record_thread->start();

    connect(recorder, SIGNAL(resultReady(QString)), this, SLOT(startDecode(QString)));

    encoder = new BtEncoder(encoder_thread);
    encoder->moveToThread(encoder_thread);
    encoder_thread->start();

    connect(this, SIGNAL(startRecord()), recorder, SLOT(start()));
    connect(this, SIGNAL(startRecord()), encoder, SLOT(start()));
    emit startRecord();
}

void BtOnline::startDecode(QString filename)
{
    QString cmd = KAL_NATO_DIR"decode2.sh ";
    cmd += filename.split('/').last();

    system(cmd.toStdString().c_str()); //init decode dir
}
