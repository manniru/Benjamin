#include "bt_chapar.h"
#include <QDebug>
#include <QDir>

BtChapar::BtChapar(BtState *st, QObject *parent) : QObject(parent)
{
    if( st->state==BT_TEST_MODE )
    {
        test = new BtTest(KAL_WAV_DIR, st);
    }
    else if( st->state==BT_ENN_MODE )
    {
//    enn = new BtEnn(KAL_AU_DIR"train/online/");
        QString train_dir = ab_getAudioPath();
        train_dir += "train/";
        createEnn(train_dir, st);
    }
    else
    {
        kaldi_thread = new QThread;
        KdOnline  *kaldi = new KdOnline(st);
        kaldi->moveToThread(kaldi_thread);
        kaldi_thread->start();

        connect(this, SIGNAL(startDecoding()), kaldi, SLOT(init()));
        emit startDecoding();
    }
}

void BtChapar::createEnn(QString dir, BtState *st)
{
    QDir p_dir(dir);
    QStringList fmt;
    fmt.append("*");
//    fmt.append("*.wav");
    QStringList dir_list = p_dir.entryList(fmt, QDir::Dirs |
                                           QDir::NoDotAndDotDot);

    for( int i=0 ; i<dir_list.size() ; i++ )
    {
        BtEnn enn(dir + dir_list[i] + "/", st);
        enn.init(dir_list[i]);
    }
    exit(0);
}

BtChapar::~BtChapar()
{
#ifdef BT_TEST_MODE
    delete test;
#else
#endif
}
