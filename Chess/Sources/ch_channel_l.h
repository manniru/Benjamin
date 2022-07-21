#ifndef CH_CHANNEL_L_H
#define CH_CHANNEL_L_H

#include <QObject>
#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "config.h"
#include "backend.h"

class ChChannelL: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", COM_NAME)
public:
    ChChannelL(QObject *parent = NULL);
    ~ChChannelL();

signals:
    void show(QString args);

public slots:
    void showUI(const QString &text);

private:
    void ConnectDBus();
};

#endif // CH_CHANNEL_L_H
