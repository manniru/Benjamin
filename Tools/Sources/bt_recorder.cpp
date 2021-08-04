#include "bt_recorder.h"
#include <QDebug>

BtRecoder::BtRecoder(QThread *thread, QObject *parent) : QObject(parent)
{
    int argc = 0;
    char **argv;
    record_timer = new QTimer;
    record_timer->moveToThread(thread);
    system(KAL_SD_DIR"init.sh"); //init decode dir

    connect(record_timer, SIGNAL(timeout()), this, SLOT(recordTimeout()));

    gst_init(&argc, &argv);

    /* Create the elements */
    source = gst_element_factory_make("pulsesrc", "source");
    filter = gst_element_factory_make ("capsfilter", "filter");
    sink = gst_element_factory_make("appsink", "sink");

    /* Create the empty pipeline */
    pipeline = gst_pipeline_new ("test-pipeline");

    if( !pipeline || !source || !sink )
    {
        g_printerr ("Not all elements could be created.\n");
    }

    /* Build the pipeline */
    gst_bin_add_many (GST_BIN(pipeline), source, filter, sink, NULL);

    gst_element_link(source, filter);
    gst_element_link(filter, sink);

    caps = gst_caps_new_simple ("audio/x-raw",
                                "format", G_TYPE_STRING, "S16LE",
                                "rate",  G_TYPE_INT, 16000,
                                "layout", G_TYPE_STRING, "interleaved",
                                "channels",G_TYPE_INT, 1, NULL);

    g_object_set (sink, "emit-signals", TRUE, "caps", caps, NULL);
    g_signal_connect (sink, "new-sample", G_CALLBACK (new_sample), NULL);

    /* we set the input filename to the source element */
    g_object_set(G_OBJECT(filter), "caps", caps, NULL);
}

BtRecoder::~BtRecoder()
{
    if (msg != NULL)
    {
      gst_message_unref (msg);
    }

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

void BtRecoder::start()
{
    /* Start playing */
    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return;
    }

    record_timer->start(5000);
}

void BtRecoder::recordTimeout()
{
    gst_element_set_state(pipeline, GST_STATE_NULL);

    emit resultReady(wav_filename);
    start();
}

/* The appsink has received a buffer */
GstFlowReturn new_sample(GstElement *sink)
{
    GstSample *sample;
    int16_t *raw;
    int16_t  raw_max = 0;

    /* Retrieve the buffer */
    g_signal_emit_by_name (sink, "pull-sample", &sample);
    if (sample)
    {
        GstBuffer *buffer = gst_sample_get_buffer(sample);

        /* Generate some psychodelic waveforms */
        GstMapInfo map;
        gst_buffer_map (buffer, &map, GST_MAP_READ);
        raw = (int16_t *)map.data;
        /* The only thing we do in this example is print a * to indicate a received buffer */

        for( int i=0 ; i<map.size ; i++ )
        {
            if( qAbs(raw[i])>raw_max )
            {
                raw_max = qAbs(raw[i]);
            }
        }

        qDebug() << map.size << raw_max << raw[0] << raw[1] << raw[2];
        gst_sample_unref (sample);
        return GST_FLOW_OK;
    }

    return GST_FLOW_ERROR;
}
