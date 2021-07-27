#include "bt_online.h"
#include "bt_recorder.h"

BtOnline::BtOnline(QObject *parent) : QObject(parent)
{
    thread_data = new RecordPipe;
    record_thread = new QThread;

    BtRecoder *recorder = new BtRecoder;
    recorder->moveToThread(record_thread);

    connect(this, SIGNAL(startRecord()), recorder, SLOT(start()));

    record_thread->start();
    emit startRecord();
}

void BtOnline::switchWindow(int index)
{
    //state->setMode(bt_MODE_HIDDEN);
}
