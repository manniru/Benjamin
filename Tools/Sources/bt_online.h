#ifndef BT_ONLINE_H
#define BT_ONLINE_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include "bt_channel_l.h"

#include "bt_state.h"

// A packet for communicating with
// the record thread
typedef struct RecordPipe
{
    QString  message;
    BtState *state;
    QStringList *wins_title;
    QStringList *elems_name;
    QVector<ReWindow> windows;
}RecordPipe;

class BtOnline : public QObject
{
    Q_OBJECT
public:
    explicit BtOnline(QObject *parent = nullptr);

private slots:
    void startDecode(QString filename);

signals:
    void startRecord();

private:
    QThread *record_thread;
    RecordPipe *thread_data;
    std::thread *api_thread;
};

void btRecordRun(void *thread_struct_void);

#endif // BT_ONLINE_H
