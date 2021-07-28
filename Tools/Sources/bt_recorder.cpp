#include "bt_recorder.h"
#include <QDebug>

BtRecoder::BtRecoder(QThread *thread, QObject *parent) : QObject(parent)
{
    int argc = 0;
    char **argv;
    record_timer = new QTimer;
    record_timer->moveToThread(thread);

    connect(record_timer, SIGNAL(timeout()), this, SLOT(recordTimeout()));

    gst_init(&argc, &argv);

    /* Create the elements */
    source = gst_element_factory_make("pulsesrc", "source");
    encoder = gst_element_factory_make("wavenc", "encoder");
    sink = gst_element_factory_make("filesink", "sink");

    /* Create the empty pipeline */
    pipeline = gst_pipeline_new ("test-pipeline");

    if (!pipeline || !source || !sink)
    {
        g_printerr ("Not all elements could be created.\n");
    }
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
    /* Build the pipeline */
    gst_bin_add_many (GST_BIN(pipeline), source, encoder, sink, NULL);

    gst_element_link(source, encoder);
    gst_element_link(encoder, sink);

    /* Modify the source's properties */
    g_object_set (sink, "location", "test.wav", NULL);

    /* Start playing */
    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return;
    }
    /* Wait until error or EOS */
    bus = gst_element_get_bus (pipeline);
    int msg_type = (GstMessageType)GST_MESSAGE_ERROR | (GstMessageType)GST_MESSAGE_EOS;

    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)msg_type);

    qDebug() << "finished";

    record_timer->start(5000);
}



void BtRecoder::recordTimeout()
{
    record_timer->stop();
    emit resultReady();
}
