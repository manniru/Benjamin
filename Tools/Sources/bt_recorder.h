#ifndef BT_RECORDER_H
#define BT_RECORDER_H

#include <QTimer>
#include <QThread>
#include <QObject>
#include <gst/gst.h>

#include "bt_config.h"

#define BT_REC_PIPELINE "playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm"

class BtRecoder : public QObject
{
    Q_OBJECT
public:
    explicit BtRecoder(QThread *thread, QObject *parent = nullptr);

    ~BtRecoder();
public slots:
    void start();

signals:
    void resultReady(QString filename);

private slots:
    void recordTimeout();

private:
    QTimer *record_timer;
    QString wav_filename;

    GstElement *pipeline;
    GstElement *source, *filter, *sink, *encoder;
    GstBus *bus;
    GstCaps *caps;
    GstMessage *msg;
    GstStateChangeReturn ret;
};

#endif // BT_RECORDER_H
