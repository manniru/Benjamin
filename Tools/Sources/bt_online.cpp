#include "bt_online.h"
#include "bt_recorder.h"

BtOnline::BtOnline(QObject *parent) : QObject(parent)
{
    thread_data = new RecordPipe;
    record_thread = new QThread;

    BtRecoder *recorder = new BtRecoder(record_thread);
    recorder->moveToThread(record_thread);

    connect(this, SIGNAL(startRecord()), recorder, SLOT(start()));
    connect(recorder, SIGNAL(resultReady(QString)), this, SLOT(startDecode(QString)));

    record_thread->start();
    emit startRecord();
}

void BtOnline::startDecode(QString filename)
{
    QString cmd = KAL_NATO_DIR"decode2.sh ";
    cmd += filename.split('/').last();
    qDebug() << cmd;

    system(cmd.toStdString().c_str()); //init decode dir
}
