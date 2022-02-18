#ifndef BT_CHAPAR_H
#define BT_CHAPAR_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>

#include "bt_captain.h"
#include "kd_online.h"
#include "bt_test.h"

class ReChapar : public QObject
{
    Q_OBJECT
public:
    explicit ReChapar(QObject *parent = nullptr);

signals:
    void startDecoding();

private:
    QThread     *kaldi_thread;
    BtCaptain   *cap;
};

#endif // BT_CHAPAR_H
