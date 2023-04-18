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
        QString train_dir = ab_getAudioPath();
#ifdef WIN32
        train_dir += "train\\";
#else
        train_dir += "train/";
#endif
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
#ifdef WIN32
        BtEnn enn(dir + dir_list[i] + "\\", st);
#else
        BtEnn enn(dir + dir_list[i] + "/", st);
#endif
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
