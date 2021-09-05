#ifndef BT_ONLINE_H
#define BT_ONLINE_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include "bt_channel_l.h"

#include "bt_state.h"
#include "bt_recorder.h"
#include "bt_encoder.h"
#include "kd_online.h"

class BtOnline : public QObject
{
    Q_OBJECT
public:
    explicit BtOnline(QObject *parent = nullptr);

private slots:
    void startDecode(QString msg);

signals:
    void startRecord();

private:
    QThread *record_thread;
    QThread *encoder_thread;
    QThread *kaldi_thread;
    BtRecoder *recorder;
    BtEncoder *encoder;
    BtCyclic  *cyclic;
    KdOnline  *kaldi;
};

void btRecordRun(void *thread_struct_void);

#endif // BT_ONLINE_H
