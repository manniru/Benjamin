#ifndef BT_ENCORDER_H
#define BT_ENCORDER_H

#include <QTimer>
#include <QThread>
#include <QObject>
#include <gst/gst.h>

#include "bt_config.h"
#include "bt_cyclic.h"

#include "kd_online2.h"

#define CHUNK_SIZE  1000   /* Amount of sample we are sending in each buffer */

class BtEncoder : public QObject
{
    Q_OBJECT
public:
    explicit BtEncoder(KdOnline2 *kaldi, BtCyclic *buffer, QThread *thread, QObject *parent = nullptr);

    ~BtEncoder();
public slots:
    void recordTimeout();
    void startEncode(QString message);

signals:
    void resultReady(QString filename);

private:
    bool pushChunk(int sample_count);

    QTimer *record_timer;
    QString wav_filename;

    GstElement *pipeline;
    GstElement *source, *queue, *sink, *encoder;
    GstBus   *bus;
    GstCaps  *caps;
    BtCyclic *cyclic;
    QString   msg;

    long wav_num = 0;
    long sample_index;   /* Number of samples generated so far (for timestamp generation) */

    KdOnline2 *kd;
    float raw_f[BT_REC_SIZE*BT_REC_RATE];
};

#endif // BT_ENCORDER_H
