#include "bt_online.h"

BtOnline::BtOnline(QObject *parent) : QObject(parent)
{
    kaldi_thread   = new QThread;

#ifdef BT_ONLINE2
    kaldi = new KdOnline2;
#else
    kaldi = new KdOnline;
#endif

    kaldi->moveToThread(kaldi_thread);
    kaldi_thread->start();
    connect(this,  SIGNAL(startRecord()), kaldi, SLOT(init()));
    emit startRecord();
}

void BtOnline::startDecode(QString msg)
{
    QString filename = msg.split(" ")[0];
//    QString cmd = "time "KAL_NATO_DIR"decode2.sh ";
    QString cmd = KAL_NATO_DIR"decode2.sh ";
    cmd += filename.split('/').last();
    cmd += " &";

    system(cmd.toStdString().c_str()); //init decode dir
//    qDebug() << "online" << msg;
}

void BtOnline::decodeOnline()
{

}
