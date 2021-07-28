#ifndef BT_RECORDER_H
#define BT_RECORDER_H

#include <QUrl>
#include <QTimer>
#include <QThread>
#include <QAudioRecorder>
#include <QObject>

#include "bt_config.h"

class BtRecoder : public QObject
{
    Q_OBJECT
public:
    explicit BtRecoder(QThread *thread, QObject *parent = nullptr);

public slots:
    void start();

signals:
    void resultReady();

private slots:
    void recordTimeout();

private:
    QAudioRecorder *recorder;
    QTimer *record_timer;
};

#endif // BT_RECORDER_H
