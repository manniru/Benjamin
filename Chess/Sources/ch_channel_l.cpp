#include "ch_channel_l.h"
#include <unistd.h>

ChChannelL::ChChannelL(QObject *parent) : QObject(parent)
{
    ConnectDBus();
}

ChChannelL::~ChChannelL()
{
    ;
}

void ChChannelL::ConnectDBus()
{
    QDBusConnection session = QDBusConnection::sessionBus();

    if( !session.isConnected() )
    {
        qFatal("Cannot connect to the D-Bus session bus.");
        return;
    }

    session.connect("", "/", COM_NAME, "show", this, SLOT(showUI(const QString &)));

    if( !session.registerService(COM_NAME) )
    {
        return;
    }
}

void ChChannelL::showUI(const QString &text)
{
    QString arg = text;
    emit show(arg);
}
