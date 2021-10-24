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
    void createPipeline();

    QString wav_filename;

    GstElement *pipeline;
    GstElement *source, *filter, *sink;
    GstBus *error_bus;
    GstCaps *caps;
    GstMessage *msg;

    BtCyclic *cy_buffer;
    long sample_count;
};

GstFlowReturn new_sample(GstElement *sink, BtRecoder *recorder);


#endif // BT_RECORDER_H
