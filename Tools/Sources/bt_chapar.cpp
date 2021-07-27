#include "bt_chapar.h"
#include <QDebug>

ReChapar::ReChapar(QObject *parent) : QObject(parent)
{
    state = new BtState;
    channel = new ReChannelL;
    online = new BtOnline;
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
