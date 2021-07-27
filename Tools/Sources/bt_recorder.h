#ifndef BT_RECORDER_H
#define BT_RECORDER_H

#include <QObject>
#include <QAudioRecorder>
#include <QUrl>
#include <QTimer>

#include "bt_config.h"

class BtRecoder : public QObject
{
    Q_OBJECT
public:
    explicit BtRecoder(QObject *parent = nullptr);

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
