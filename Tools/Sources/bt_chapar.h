#ifndef BT_CHAPAR_H
#define BT_CHAPAR_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>

#include "bt_state.h"
#include "bt_captain.h"

#ifdef BT_ONLINE2
#include "kd_online2.h"
#else
#include "kd_online.h"
#endif

class ReChapar : public QObject
{
    Q_OBJECT
public:
    explicit ReChapar(QObject *parent = nullptr);

public slots:
    void execute(const QString &text);

private slots:
    void switchWindow(int index);
    void requstSuspend();

signals:
    void startDecoding();

private:
    QThread     *kaldi_thread;
    BtState     *state;
    BtCaptain   *cap;
};

#endif // BT_CHAPAR_H
