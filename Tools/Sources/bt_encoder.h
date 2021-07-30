#ifndef BT_ENCORDER_H
#define BT_ENCORDER_H

#include <QTimer>
#include <QThread>
#include <QObject>
#include <gst/gst.h>

#include "bt_config.h"

#define CHUNK_SIZE  1024   /* Amount of bytes we are sending in each buffer */
#define SAMPLE_RATE 16000 /* Samples per second we are sending */

typedef struct _CustomData
{
    GstElement *app_source;

    guint64 num_samples;   /* Number of samples generated so far (for timestamp generation) */
    gfloat a, b, c, d;     /* For waveform generation */

    guint sourceid;        /* To control the GSource */
} CustomData;

class BtEncoder : public QObject
{
    Q_OBJECT
public:
    explicit BtEncoder(QThread *thread, QObject *parent = nullptr);

    ~BtEncoder();
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
    GstElement *source, *queue, *sink, *encoder;
    GstBus *bus;
    GstCaps *caps;
    GstMessage *msg;
    GstStateChangeReturn ret;


    CustomData data;
    GstCaps *audio_caps;
};

static gboolean push_data (CustomData *data);
static void start_feed (GstElement *source, guint size, CustomData *data);
static void stop_feed (GstElement *source, CustomData *data);
static void error_cb (GstBus *bus, GstMessage *msg, CustomData *data);


#endif // BT_ENCORDER_H
