#include "ch_chapar.h"

ChChapar::ChChapar(QObject *ui, QObject *parent) : QObject(parent)
{
#ifdef WIN32
    ch_thread = new QThread;
    ChChannelW *channel = new ChChannelW;
    channel->moveToThread(ch_thread);
    ch_thread->start();
    ChProcessorW *processor = new ChProcessorW(channel, ui);

    connect(ui, SIGNAL(eKeyPressed(int)),
            processor, SLOT(keyPressed(int)));

    connect(this, SIGNAL(run()),
            channel, SLOT(listenPipe()));

    emit run();

#else
    ChChannelL *channel = new ChChannelL;
    ChProcessorL *processor = new ChProcessorL(channel, root);

    QObject::connect(root, SIGNAL(eKeyPressed(int)),
                     processor, SLOT(keyPressed(int)));
#endif
}
