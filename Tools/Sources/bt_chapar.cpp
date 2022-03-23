#include "bt_chapar.h"
#include <QDebug>

ReChapar::ReChapar(QObject *parent) : QObject(parent)
{
    kaldi_thread = new QThread;

#ifdef BT_TEST_MODE
    BtTest *test = new BtTest(KAL_WAV_DIR"rec_test.wav");
#else
    KdOnline  *kaldi = new KdOnline;
    kaldi->moveToThread(kaldi_thread);
    kaldi_thread->start();

    connect(this, SIGNAL(startDecoding()), kaldi, SLOT(init()));
    emit startDecoding();
#endif
}
