#include "bt_recorder.h"
#include <QDebug>

BtRecoder::BtRecoder(QThread *thread, BtCyclic *buffer, QObject *parent) : QObject(parent)
{
    int argc = 0;
    char **argv;
    system(KAL_SD_DIR"init.sh"); //init decode dir

    gst_init(&argc, &argv);
    sample_count = 0;
    cy_buffer = buffer;
//    cy_buffer->constWrite(0, (BT_REC_SIZE-BT_DEC_TIMEOUT)*BT_REC_RATE);

    /* Create the elements */
    source = gst_element_factory_make("pulsesrc"  , "source");
    filter = gst_element_factory_make("capsfilter", "filter");
    sink   = gst_element_factory_make("appsink"   , "sink");

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
                                "format" , G_TYPE_STRING, "S16LE",
                                "rate"   , G_TYPE_INT,    BT_REC_RATE,
                                "layout" , G_TYPE_STRING, "interleaved",
                                "channels",G_TYPE_INT,    1, NULL);

    g_object_set (sink, "emit-signals", TRUE, "caps", caps, NULL);
    g_signal_connect (sink, "new-sample", G_CALLBACK (new_sample), this);

    g_object_set (source, "buffer-time", 10000, NULL);

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

//    record_timer->start(BT_DEC_TIMEOUT*1000);
}

void BtRecoder::bufferReady(QString message)
{
    sample_count = 0;
    emit resultReady(message);
}

long BtRecoder::addSample(int16_t *data, int count)
{
    cy_buffer->write(data, count);
    sample_count += count;

    return sample_count;
}

/* The appsink has received a buffer */
GstFlowReturn new_sample(GstElement *sink, BtRecoder *recorder)
{
    GstSample *sample;
    int16_t *raw;

    /* Retrieve the buffer */
    g_signal_emit_by_name (sink, "pull-sample", &sample);
    if (sample)
    {
        GstBuffer *buffer = gst_sample_get_buffer(sample);

        GstMapInfo map;
        gst_buffer_map(buffer, &map, GST_MAP_READ);
        raw = (int16_t *)map.data;
        // Size half because of 16bit data.
        long sample_index = recorder->addSample(raw, map.size/2);

        for( int i=0 ; i<map.size ; i++ )
        {
            recorder->sum_avg += qAbs(raw[i]);
        }

        gst_sample_unref (sample);

        if( sample_index>=BT_DEC_TIMEOUT*BT_REC_RATE )
        {
            QString message = QTime::currentTime().toString("mm:ss:zzz") + " ";
            double max_amp = 20*qLn(recorder->sum_avg/sample_index)/qLn(10);
            message += QString::number(max_amp);

//            qDebug() << "recorder" << message << sample_index;;
            recorder->bufferReady(message);
            recorder->sum_avg = 0;
        }

        return GST_FLOW_OK;
    }

    return GST_FLOW_ERROR;
}
