#ifndef BT_CHAPAR_H
#define BT_CHAPAR_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>

#include "bt_captain.h"
#include "kd_online.h"
#include "bt_test.h"
#include "bt_enn.h"
#include "bt_state.h"

class BtChapar : public QObject
{
    Q_OBJECT
public:
    explicit BtChapar(BtState *st, QObject *parent = nullptr);
    ~BtChapar();

signals:
    void startDecoding();

private:
    void createEnn(QString dir, BtState *st);

    QThread *kaldi_thread;
    BtTest  *test;
};

#endif // BT_CHAPAR_H
