#ifndef RECHAPAR_H
#define RECHAPAR_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include "bt_channel_l.h"

#include "bt_state.h"

class ReChapar : public QObject
{
    Q_OBJECT
public:
    explicit ReChapar(QObject *parent = nullptr);

private slots:
    void updateMode();
    void switchWindow(int index);
    void requstSuspend();

private:
    ReState    *state;
    ReChannelL *channel;
};

#endif // RECHAPAR_H
