#include "bt_channel_l.h"
#include <unistd.h>

ReChannelL::ReChannelL(QObject *parent) : QObject(parent)
{
    ConnectDBus();
}

ReChannelL::~ReChannelL()
{
    ;
}

void ReChannelL::ConnectDBus()
{
    QDBusConnection session = QDBusConnection::sessionBus();

    if (!session.isConnected())
    {
        qFatal("Cannot connect to the D-Bus session bus.");
        return;
    }

    session.connect("", "/", DBUS_NAME, "nato" , this, SLOT(nato (const QString &)));
    session.connect("", "/", DBUS_NAME, "exec"  , this, SLOT(execute()));

    if(!session.registerService(DBUS_NAME))
    {
        qFatal("Another session is on DBus.");
        // This cannot be automatic because killing would also kill
        // this instant too
        return;
    }
}

void ReChannelL::execute()
{
    ;
}

void ReChannelL::nato(const QString &text)
{
    ;
}

void ReChannelL::startServer()
{

}
