#ifndef BT_CHAPAR_H
#define BT_CHAPAR_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>

#include "bt_channel_l.h"
#include "bt_online.h"
#include "bt_state.h"

class ReChapar : public QObject
{
    Q_OBJECT
public:
    explicit ReChapar(QObject *parent = nullptr);

private slots:
    void switchWindow(int index);
    void requstSuspend();

private:
    BtOnline   *online;
    BtState    *state;
    ReChannelL *channel;
};

#endif // BT_CHAPAR_H
