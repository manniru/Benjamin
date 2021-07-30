#include "bt_encoder.h"
#include <QDebug>

BtEncoder::BtEncoder(QThread *thread, QObject *parent) : QObject(parent)
{
    int argc = 0;
    char **argv;
    record_timer = new QTimer;
    record_timer->moveToThread(thread);

    connect(record_timer, SIGNAL(timeout()), this, SLOT(recordTimeout()));

    /* Initialize custom data structure */
    memset (&data, 0, sizeof (data));
    data.b = 1; /* For waveform generation */
    data.d = 1;

    /* Create the elements */
    data.app_source     = gst_element_factory_make ("appsrc", "audio_source");
    queue   = gst_element_factory_make ("queue", "audio_queue");
    encoder = gst_element_factory_make("wavenc", "encoder");
    sink    = gst_element_factory_make ("filesink", "sink");

    g_object_set (sink, "location", "test.wav", NULL);

    /* Create the empty pipeline */
    pipeline = gst_pipeline_new ("test-pipeline");

    if (!pipeline || !data.app_source || !queue || !sink )
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

    g_object_set (data.app_source, "caps", audio_caps, "format", GST_FORMAT_TIME, NULL);
    g_signal_connect (data.app_source, "need-data", G_CALLBACK (start_feed), &data);
    g_signal_connect (data.app_source, "enough-data", G_CALLBACK (stop_feed), &data);

    gst_caps_unref (audio_caps);

    /* Link all elements that can be automatically linked because they have "Always" pads */
    gst_bin_add_many (GST_BIN(pipeline), data.app_source, queue, encoder, sink, NULL);

    if (gst_element_link_many(data.app_source, queue, encoder, sink, NULL) != TRUE)
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
    record_timer->start(500);
}

void BtEncoder::recordTimeout()
{
    gst_element_set_state(pipeline, GST_STATE_NULL);

    qDebug() << "finished" << wav_filename;
    emit resultReady(wav_filename);
//    start();
}

static gboolean push_data (CustomData *data)
{
    GstBuffer *buffer;
    GstMapInfo map;
    int16_t *raw;

    int i;
    gint num_samples = CHUNK_SIZE / 2; /* Because each sample is 16 bits */
    gfloat freq;
    GstFlowReturn ret;
    /* Create a new empty buffer */
    buffer = gst_buffer_new_and_alloc (CHUNK_SIZE);

    /* Set its timestamp and duration */
    GST_BUFFER_TIMESTAMP (buffer) = gst_util_uint64_scale (data->num_samples, GST_SECOND, SAMPLE_RATE);
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale (num_samples, GST_SECOND, SAMPLE_RATE);

    /* Generate some psychodelic waveforms */
    gst_buffer_map (buffer, &map, GST_MAP_WRITE);
    raw = (int16_t *)map.data;
    data->c += data->d;
    data->d -= data->c / 1000;
    freq = 1100 + 1000 * data->d;

    for ( i=0 ; i<num_samples ; i++ )
    {
        data->a += data->b;
        data->b -= data->a / freq;
        raw[i]   = (int16_t)(500 * data->a);
    }

    gst_buffer_unmap (buffer, &map);
    data->num_samples += num_samples;

    /* Push the buffer into the appsrc */
    g_signal_emit_by_name (data->app_source, "push-buffer", buffer, &ret);

    /* Free the buffer now that we are done with it */
    gst_buffer_unref (buffer);

    if (ret != GST_FLOW_OK)
    {
        return FALSE;
    }

    return TRUE;
}

/* This signal callback triggers when appsrc needs data. Here, we add an idle handler
 * to the mainloop to start pushing data into the appsrc */
static void start_feed (GstElement *source, guint size, CustomData *data)
{
  if (data->sourceid == 0) {
    g_print ("Start feeding\n");
    data->sourceid = g_idle_add ((GSourceFunc) push_data, data);
  }
}

/* This callback triggers when appsrc has enough data and we can stop sending.
 * We remove the idle handler from the mainloop */
static void stop_feed (GstElement *source, CustomData *data)
{
  if (data->sourceid != 0) {
    g_print ("Stop feeding\n");
    g_source_remove (data->sourceid);
    data->sourceid = 0;
  }
}

/* This function is called when an error message is posted on the bus */
static void error_cb (GstBus *bus, GstMessage *msg, CustomData *data)
{
  GError *err;
  gchar *debug_info;

  /* Print error details on the screen */
  gst_message_parse_error (msg, &err, &debug_info);
  g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
  g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
  g_clear_error (&err);
  g_free (debug_info);
}
