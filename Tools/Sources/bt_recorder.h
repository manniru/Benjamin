#ifndef BT_RECORDER_H
#define BT_RECORDER_H

#include <QTime>
#include <QtMath>
#include <QTimer>
#include <QThread>
#include <QObject>
#include <gst/gst.h>

#include "bt_config.h"
#include "bt_cyclic.h"

#define BT_REC_PIPELINE "playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm"

class BtRecoder : public QObject
{
    Q_OBJECT
public:
    explicit BtRecoder(QThread *thread, BtCyclic *buffer, QObject *parent = nullptr);

    long addSample(int16_t *data, int count);
    void bufferReady(QString message);

    long sum_avg;
    ~BtRecoder();
public slots:
    void start();

signals:
    void resultReady(QString message);


private:
    QString wav_filename;

    GstElement *pipeline;
    GstElement *source, *filter, *sink;
    GstBus *bus;
    GstCaps *caps;
    GstMessage *msg;
    GstStateChangeReturn ret;

    BtCyclic *cy_buffer;
    long sample_count;
};

GstFlowReturn new_sample(GstElement *sink, BtRecoder *recorder);


#endif // BT_RECORDER_H
