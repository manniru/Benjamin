#ifndef BT_ONLINE_H
#define BT_ONLINE_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include "bt_channel_l.h"

#include "bt_state.h"
#ifdef BT_ONLINE2
#include "kd_online2.h"
#else
#include "kd_online.h"
#endif

class BtOnline : public QObject
{
    Q_OBJECT
public:
    explicit BtOnline(QObject *parent = nullptr);

private slots:
    void startDecode(QString msg);
    void decodeOnline();

signals:
    void startRecord();

private:
    QThread   *kaldi_thread;
    BtCyclic  *cyclic;
#ifdef BT_ONLINE2
    KdOnline2 *kaldi;
#else
    KdOnline  *kaldi;
#endif
};

void btRecordRun(void *thread_struct_void);

#endif // BT_ONLINE_H
