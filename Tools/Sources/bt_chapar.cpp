#include "bt_chapar.h"
#include <QDebug>

ReChapar::ReChapar(QObject *parent) : QObject(parent)
{
#ifdef BT_TEST_MODE
    test = new BtTest(KAL_WAV_DIR);
#else
    kaldi_thread = new QThread;
    KdOnline  *kaldi = new KdOnline;
    kaldi->moveToThread(kaldi_thread);
    kaldi_thread->start();

    connect(this, SIGNAL(startDecoding()), kaldi, SLOT(init()));
    emit startDecoding();
#endif
}

ReChapar::~ReChapar()
{
#ifdef BT_TEST_MODE
    delete test;
#else
#endif
}
