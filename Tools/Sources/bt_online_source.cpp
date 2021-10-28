#include "bt_online_source.h"

BtOnlineSource::BtOnlineSource(BtCyclic *buffer)
{
    cy_buf = buffer;

    PaError pa_err = Pa_Initialize();
    if( pa_err )
    {
        qDebug() << "PortAudio initialization error";
    }

    pa_err = Pa_OpenDefaultStream(&pa_stream, 1, 0, paInt16
                         , BT_REC_RATE, 0, PaCallback, this);
    if( pa_err )
    {
        qDebug() << "PortAudio failed to open the default stream";
        pa_stream = NULL;
    }
}

BtOnlineSource::~BtOnlineSource()
{
    if( pa_stream )
    {
        Pa_StopStream(pa_stream);
    }
    if( pa_stream )
    {
        Pa_CloseStream(pa_stream);
        Pa_Terminate();
    }
}

// cy_buf will be filled from the other thread using GStreamer
bool BtOnlineSource::Read(kaldi::Vector<float> *data)
{
    if( Pa_IsStreamStopped(pa_stream) )
    {
        PaError paerr = Pa_StartStream(pa_stream);
        if( paerr )
        {
            qDebug() << "Error while trying to open PortAudio stream";
            exit(0);
        }
    }
    int req = data->Dim();  // number of samples to request
    int16_t *raw = (int16_t *)malloc(sizeof(int16_t)*req);

    while( true )
    {
        QThread::msleep(2);
        if( req<cy_buf->getDataSize() )
        {
            break;
        }
    }

    int nsamples = cy_buf->read(raw, req);
    data->Resize(nsamples);
//    qDebug() << req << nsamples;

    for( int i = 0 ; i<nsamples ; i++ )
    {
        (*data)(i) = raw[i];
    }
    free(raw);

    if( nsamples==0 )
    {
        return false;
    }

    return true;
}

int BtOnlineSource::Callback(int16_t *data, int size)
{
    if( cy_buf->getFreeSize()<size )
    {
        qDebug() << "Overflow occurd" << size << cy_buf->getFreeSize();
        return paContinue;
    }
    cy_buf->write(data, size);
    return paContinue;
}

int PaCallback(const void *input, void *output,
               long unsigned frame_count,
               const PaStreamCallbackTimeInfo *time_info,
               PaStreamCallbackFlags status_flags, void *user_data)
{
    BtOnlineSource *pa_src = reinterpret_cast<BtOnlineSource*>(user_data);

    int size = frame_count;
    void *ptr = const_cast<void *>(input);

    return pa_src->Callback((int16_t *)ptr, size);
}
