#ifndef BT_CHANNEL_L_H
#define BT_CHANNEL_L_H

#include "backend.h"
#include "bt_config.h"
#include "bt_captain.h"

#include <QObject>
#include <QtCore/QObject>
#include <QtDBus/QtDBus>

class ReChannelL : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DBUS_NAME)
public:
    ReChannelL(QObject *parent = NULL);

    void startServer();

    ~ReChannelL();

public slots:
    void nato (const QString &text);
    void execute(const QString &text);

private:
    void ConnectDBus();

    BtCaptain *cap;
};

#endif // BT_CHANNEL_L_H
