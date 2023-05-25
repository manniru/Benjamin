#include "ch_chapar.h"

ChChapar::ChChapar(QObject *ui, QObject *parent) : QObject(parent)
{
#ifdef WIN32
    ch_thread = new QThread;
    channel = new ChChannelW;
    channel->moveToThread(ch_thread);
    ch_thread->start();
    processor = new ChProcessorW(ui);

    connect(channel, SIGNAL(show_chess(QString)),
            processor, SLOT(showUI(QString)));
    connect(channel, SIGNAL(meta()),
            processor, SLOT(meta()));
    connect(channel, SIGNAL(cancel()),
            processor, SLOT(cancel()));
    connect(channel, SIGNAL(key_chess(int)),
            processor, SLOT(keyReceived(int)));

    connect(this, SIGNAL(run()),
            channel, SLOT(listenPipe()));

    emit run();

#else
    ChChannelL *channel = new ChChannelL;
    ChProcessorL *processor = new ChProcessorL(channel, ui);

    QObject::connect(ui, SIGNAL(eKeyPressed(int)),
                     processor, SLOT(key(int)));
#endif
}
