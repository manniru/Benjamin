#include "bt_online.h"

BtOnline::BtOnline(QObject *parent) : QObject(parent)
{
    record_thread = new QThread;
    encoder_thread = new QThread;
    cyclic = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);

    recorder = new BtRecoder(record_thread, cyclic);
    recorder->moveToThread(record_thread);
    record_thread->start();


    encoder = new BtEncoder(encoder_thread, cyclic);
    encoder->moveToThread(encoder_thread);
    encoder_thread->start();

    connect(recorder, SIGNAL(resultReady()), encoder, SLOT(startEncode()));
    connect(this, SIGNAL(startRecord()), recorder, SLOT(start()));
    connect(encoder, SIGNAL(resultReady(QString)), this, SLOT(startDecode(QString)));
    emit startRecord();
}

void BtOnline::startDecode(QString filename)
{
    QString cmd = KAL_NATO_DIR"decode2.sh ";
    cmd += filename.split('/').last();

    system(cmd.toStdString().c_str()); //init decode dir
}
