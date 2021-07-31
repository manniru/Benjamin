#include "bt_encoder.h"
#include <QDebug>

BtEncoder::BtEncoder(QThread *thread, QObject *parent) : QObject(parent)
{
    record_timer = new QTimer;
    record_timer->moveToThread(thread);
    record_timer->setSingleShot(true);

    connect(record_timer, SIGNAL(timeout()), this, SLOT(recordTimeout()));

    /* Initialize custom data structure */
    memset (&data, 0, sizeof (data));
    data.b = 1; /* For waveform generation */
    data.d = 1;

    /* Create the elements */
    source  = gst_element_factory_make ("appsrc", "source");
    queue   = gst_element_factory_make ("queue", "queue");
    encoder = gst_element_factory_make("wavenc", "encoder");
    sink    = gst_element_factory_make ("filesink", "sink");

    g_object_set (sink, "location", "test.wav", NULL);

    /* Create the empty pipeline */
    pipeline = gst_pipeline_new ("test-pipeline");

    if (!pipeline || !source || !queue || !sink )
    {
        g_printerr ("Not all elements could be created.\n");
        return;
    }

    /* Configure appsrc */
    audio_caps = gst_caps_new_simple ("audio/x-raw",
                                      "format", G_TYPE_STRING, "S16LE",
                                      "rate",  G_TYPE_INT, SAMPLE_RATE,
                                      "layout", G_TYPE_STRING, "interleaved",
                                      "channels",G_TYPE_INT, 1, NULL);

    g_object_set (source, "caps", audio_caps, "format", GST_FORMAT_TIME, NULL);
//    g_signal_connect (data.app_source, "need-data", G_CALLBACK (start_feed), &data);
//    g_signal_connect (data.app_source, "enough-data", G_CALLBACK (stop_feed), &data);

    gst_caps_unref (audio_caps);

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
    if (msg != NULL)
    {
        gst_message_unref (msg);
    }

    gst_object_unref(bus);

    /* Free resources */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
}

void BtEncoder::start()
{
    /* Start playing the pipeline */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    /* Create a new empty buffer */
//    buffer = gst_buffer_new_and_alloc (CHUNK_SIZE);
    for( int i=0 ; i<500 ; i++ )
    {
        if( pushChunk()==false )
        {
            qDebug() << "shit has happened";
            break;
        }
    }

    /* Free the buffer now that we are done with it */
//    gst_buffer_unref (buffer);

    record_timer->start(500);
}

bool BtEncoder::pushChunk()
{
    int16_t *raw;

    int i;
    gint num_samples = CHUNK_SIZE / 2; /* Because each sample is 16 bits */
    gfloat freq;
    GstFlowReturn ret;
    buffer = gst_buffer_new_and_alloc (CHUNK_SIZE);


    /* Set its timestamp and duration */
    GST_BUFFER_TIMESTAMP (buffer) = gst_util_uint64_scale (data.num_samples, GST_SECOND, SAMPLE_RATE);
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale (num_samples, GST_SECOND, SAMPLE_RATE);

    /* Generate some psychodelic waveforms */
    GstMapInfo map;
    gst_buffer_map (buffer, &map, GST_MAP_WRITE);
    raw = (int16_t *)map.data;

    data.c += data.d;
    data.d -= data.c / 1000;
    freq = 1100 + 1000 * data.d;

    for ( i=0 ; i<num_samples ; i++ )
    {
        data.a += data.b;
        data.b -= data.a / freq;
        raw[i]  = (int16_t)(500 * data.a);
    }

    gst_buffer_unmap (buffer, &map);
    data.num_samples += num_samples;

    /* Push the buffer into the appsrc */
    g_signal_emit_by_name(source, "push-buffer", buffer, &ret);

    gst_buffer_unref (buffer);
    if (ret != GST_FLOW_OK)
    {
        return FALSE;
    }

    return TRUE;
}

void BtEncoder::recordTimeout()
{
    gst_element_set_state(pipeline, GST_STATE_NULL);

    qDebug() << "finished" << wav_filename;
    emit resultReady(wav_filename);
//    start();
}
