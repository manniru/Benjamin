#include "bt_chapar.h"
#include <QDebug>

ReChapar::ReChapar(QObject *parent) : QObject(parent)
{
    cap = new BtCaptain;
    kaldi_thread = new QThread;


    KdOnline  *kaldi = new KdOnline;
    kaldi->moveToThread(kaldi_thread);
    kaldi_thread->start();

    ///Magic Happens Here
    qRegisterMetaType<QVector<BtWord> >("QVector<BtWord>");
    connect(kaldi, SIGNAL(resultReady(QVector<BtWord>)),
            cap, SLOT(parse(QVector<BtWord>)));
    connect(this, SIGNAL(startDecoding()), kaldi, SLOT(init()));
    emit startDecoding();
}
