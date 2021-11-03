#include "bt_chapar.h"
#include <QDebug>

ReChapar::ReChapar(QObject *parent) : QObject(parent)
{
    state = new BtState;
    cap = new BtCaptain;
    kaldi_thread = new QThread;


#ifdef BT_ONLINE2
    KdOnline2 *kaldi = new KdOnline2;
#else
    KdOnline  *kaldi = new KdOnline;
#endif
    kaldi->moveToThread(kaldi_thread);
    kaldi_thread->start();

    ///Magic Happens Here
    qRegisterMetaType<QVector<BtWord> >("QVector<BtWord>");
    connect(kaldi, SIGNAL(resultReady(QVector<BtWord>)),
            cap, SLOT(parse(QVector<BtWord>)));
    connect(this, SIGNAL(startDecoding()), kaldi, SLOT(init()));
    emit startDecoding();
}

void ReChapar::switchWindow(int index)
{
    //state->setMode(bt_MODE_HIDDEN);
}

void ReChapar::requstSuspend()
{
#ifdef _WIN32
    state->hardware->disconnectXbox();
#endif
}

void ReChapar::execute(const QString &words)
{
    if( cap->isValidUtterance() )
    {
//        conf->printWords(words);

        QString cmd = KAL_SI_DIR"main.sh \"";
        cmd += cap->getUtterance() + "\"";

//        qDebug() << cmd;

        system(cmd.toStdString().c_str());
    }
    else if( cap->getUtterance().isEmpty() )
    {
        system("dbus-send --session --dest=com.binaee.rebound / "
               "com.binaee.rebound.exec  string:\"\"");
    }
}
