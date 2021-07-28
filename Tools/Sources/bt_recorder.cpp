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
    encoder = gst_element_factory_make("wavenc", "encoder");
    sink = gst_element_factory_make("filesink", "sink");

    /* Create the empty pipeline */
    pipeline = gst_pipeline_new ("test-pipeline");

    if (!pipeline || !source || !sink)
    {
        g_printerr ("Not all elements could be created.\n");
    }

    /* Build the pipeline */
    gst_bin_add_many (GST_BIN(pipeline), source, filter, encoder, sink, NULL);

    gst_element_link(source, filter);
    gst_element_link(filter, encoder);
    gst_element_link(encoder, sink);

    caps = gst_caps_new_simple ("audio/x-raw",
                                "format", G_TYPE_STRING, "S16LE",
                                "rate",  G_TYPE_INT, 16000,
                                "channels",G_TYPE_INT, 1, NULL);

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
    if( wav_filename.contains("buf2.wav") )
    {
        wav_filename = KAL_WAV_DIR"buf1.wav";
    }
    else
    {
        wav_filename = KAL_WAV_DIR"buf2.wav";
    }

    /* Modify the source's properties */
    g_object_set (sink, "location", wav_filename.toStdString().c_str(), NULL);

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

//    bus = gst_element_get_bus(pipeline);
//    int msg_type = GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS;
//    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)msg_type);

    qDebug() << "finished" << wav_filename;
    emit resultReady(wav_filename);
    start();
}
