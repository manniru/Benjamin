#ifndef BT_CHAPAR_H
#define BT_CHAPAR_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>

#include "bt_captain.h"
#include "kd_online.h"
#include "bt_test.h"
#include "bt_enn.h"

class ReChapar : public QObject
{
    Q_OBJECT
public:
    explicit ReChapar(QObject *parent = nullptr);
    ~ReChapar();

signals:
    void startDecoding();

private:
    QThread *kaldi_thread;
    BtTest  *test;
    BtEnn *enn;
};

#endif // BT_CHAPAR_H
