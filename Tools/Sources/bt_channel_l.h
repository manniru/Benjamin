#ifndef CHANNEL_H_L
#define CHANNEL_H_L

#include "backend.h"
#include "bt_config.h"
#include "bt_confidence.h"

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

    BtConfidence *conf;
};

#endif // CHANNEL_H_L
