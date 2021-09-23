#include "bt_encoder.h"
#include <QDebug>

BtEncoder::BtEncoder(QThread *thread, BtCyclic *buffer, QObject *parent) : QObject(parent)
{
    record_timer = new QTimer;
    record_timer->moveToThread(thread);
    record_timer->setSingleShot(true);
    cyclic = buffer;

    connect(record_timer, SIGNAL(timeout()), this, SLOT(recordTimeout()));

    /* Create the elements */
    source  = gst_element_factory_make ("appsrc", "source");
    queue   = gst_element_factory_make ("queue", "queue");
    encoder = gst_element_factory_make("wavenc", "encoder");
    sink    = gst_element_factory_make ("filesink", "sink");

    /* Create the empty pipeline */
    pipeline = gst_pipeline_new ("test-pipeline");

    if (!pipeline || !source || !queue || !sink )
    {
        g_printerr ("Not all elements could be created.\n");
        return;
    }

    /* Configure appsrc */
    caps = gst_caps_new_simple ("audio/x-raw",
                                      "format", G_TYPE_STRING, "S16LE",
                                      "rate",  G_TYPE_INT, BT_REC_RATE,
                                      "layout", G_TYPE_STRING, "interleaved",
                                      "channels",G_TYPE_INT, 1, NULL);

    g_object_set (source, "caps", caps, "format", GST_FORMAT_TIME, NULL);
    gst_caps_unref (caps);

    /* Link all elements that can be automatically linked because they have "Always" pads */
    gst_bin_add_many (GST_BIN(pipeline), source, queue, encoder, sink, NULL);

    if (gst_element_link_many(source, queue, encoder, sink, NULL) != TRUE)
    {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref (pipeline);
        return;
    }
}

BtEncoder::~BtEncoder()
{
    gst_object_unref(bus);

    /* Free resources */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
}

void BtEncoder::startEncode(QString message)
{
#ifdef BT_DOUBLE_BUF
    if( wav_filename.contains("buf2.wav") )
    {
        wav_filename = KAL_WAV_DIR"buf1.wav";
    }
    else
    {
        wav_filename = KAL_WAV_DIR"buf2.wav";
    }
#else
    wav_filename = KAL_WAV_DIR"buf" + QString::number(wav_num++) + ".wav";
#endif

    g_object_set (sink, "location", wav_filename.toStdString().c_str(), NULL);

    sample_index = 0;

    /* Start playing the pipeline */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    ///FIXME: Implement GstBufferPool
    int buf_count = BT_REC_SIZE*BT_REC_RATE/CHUNK_SIZE;

#ifdef BT_ONLINE2
    int sample_count = BT_REC_SIZE*BT_REC_RATE;
    cyclic->rewind((BT_REC_SIZE-BT_DEC_TIMEOUT)*BT_REC_RATE);
    int16_t raw[BT_REC_SIZE*BT_REC_RATE];
    cyclic->raad(raw, sample_count);

    for( int i=0 ; i<sample_count ; i++ )
    {
        raw_f[i] = raw[i];
    }
    kd.processData(raw_f, sample_count);
#else
    cyclic->rewind((BT_REC_SIZE-BT_DEC_TIMEOUT)*BT_REC_RATE);
    for( int i=0 ; i<buf_count ; i++ )
    {
        if( pushChunk(CHUNK_SIZE)==false )
        {
            qDebug() << "shit has happened";
            break;
        }
    }
#endif

    record_timer->start(50);

    msg = message;
}

bool BtEncoder::pushChunk(int sample_count)
{
    GstFlowReturn ret;
    GstBuffer *en_buffer = gst_buffer_new_and_alloc (sample_count*sizeof(int16_t));

    /* Set its timestamp and duration */
    GST_BUFFER_TIMESTAMP (en_buffer) = gst_util_uint64_scale (sample_index, GST_SECOND, BT_REC_RATE);
    GST_BUFFER_DURATION (en_buffer) = gst_util_uint64_scale (sample_count, GST_SECOND, BT_REC_RATE);

    /* Generate some psychodelic waveforms */
    GstMapInfo map;
    gst_buffer_map (en_buffer, &map, GST_MAP_WRITE);
    int16_t *raw = (int16_t *)map.data;

    cyclic->raad(raw, sample_count);

    gst_buffer_unmap (en_buffer, &map);
    sample_index += sample_count;

    /* Push the buffer into the appsrc */
    g_signal_emit_by_name(source, "push-buffer", en_buffer, &ret);

    gst_buffer_unref (en_buffer);
    if (ret != GST_FLOW_OK)
    {
        return FALSE;
    }

    return TRUE;
}

void BtEncoder::recordTimeout()
{
    gst_element_set_state(pipeline, GST_STATE_NULL);

    uint q_buf_count = 0;
    uint s_buf_count = 0;
    g_object_get(queue, "current-level-bytes", &q_buf_count, NULL);
    g_object_get(source, "current-level-bytes", &s_buf_count, NULL);

//    qDebug() << "done" << msg << q_buf_count << s_buf_count;

    emit resultReady(wav_filename + " " + msg);
}
