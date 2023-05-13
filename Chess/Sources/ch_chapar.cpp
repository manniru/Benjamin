#include "ch_chapar.h"

ChChapar::ChChapar(QObject *ui, QObject *parent) : QObject(parent)
{
#ifdef WIN32
    ch_thread = new QThread;
    channel = new ChChannelW;
    processor = new ChProcessorW(ui);

    connect(channel, SIGNAL(show(QString)),
            processor, SLOT(showUI(QString)));

    connect(ui, SIGNAL(eKeyPressed(int)),
            processor, SLOT(keyPressed(int)));

    connect(this, SIGNAL(run()),
            channel, SLOT(listenPipe()));

    channel->moveToThread(ch_thread);
    ch_thread->start();
    emit run();

#else
    ChChannelL *channel = new ChChannelL;
    ChProcessorL *processor = new ChProcessorL(channel, ui);

    QObject::connect(ui, SIGNAL(eKeyPressed(int)),
                     processor, SLOT(keyPressed(int)));
#endif
}
