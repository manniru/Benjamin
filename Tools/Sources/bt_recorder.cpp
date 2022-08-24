#include "bt_recorder.h"
#include <fcntl.h>
#include <unistd.h>

BtRecorder::BtRecorder(BtCyclic *buffer, QObject *parent): QObject(parent)
{
    cy_buf  = buffer;
    read_pf = 0;

#ifdef __linux__
    // Done to skip shitty alsa messages
    int saved_stdout = dup(STDERR_FILENO);
    int devnull = open("/dev/null", O_RDWR);
    dup2(devnull, STDERR_FILENO);  // Replace standard out
#endif
    PaError pa_err = Pa_Initialize();
#ifdef __linux__
    dup2(saved_stdout, STDERR_FILENO);
#endif
    if( pa_err )
    {
        qDebug() << "PortAudio initialization error";
    }

    openMic();

    callback_time = BT_INV_TIME;
    con_timer = new QTimer();
    connect(con_timer, SIGNAL(timeout()), this, SLOT(conTimeOut()));
    con_timer->start(BT_CON_TIMER);
}

BtRecorder::~BtRecorder()
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

void BtRecorder::openMic()
{
    PaError pa_err = Pa_OpenDefaultStream(&pa_stream, 1, 0, paInt16
                         , BT_REC_RATE, 0, PaCallback, this);
    if( pa_err )
    {
        qDebug() << "PortAudio failed to open the default stream";
        pa_stream = NULL;
    }
}

// This function deosn't need to be called
// Only use if you don't plan on using read function
void BtRecorder::startStream()
{
    if( Pa_IsStreamStopped(pa_stream) )
    {
        PaError paerr = Pa_StartStream(pa_stream);
        if( paerr )
        {
            qDebug() << "Failed to open PortAudio stream";
        }
    }
}

void BtRecorder::conTimeOut()
{
    if( callback_time==BT_INV_TIME )
    {
        return;
    }

    clock_t now = clock();
    int diff = now - callback_time;

    if( diff>BT_CON_TIMER )
    {
//        qDebug() << "diff:" << diff
//                 << "start:" << callback_time
//                 << "end" << now;
        if( pa_stream )
        {
            Pa_CloseStream(pa_stream);
        }
        openMic();
        startStream();
    }
}

int BtRecorder::Callback(int16_t *data, int size)
{
    if( cy_buf->getFreeSize()<size )
    {
        qDebug() << "BtRecorder Overflow" << size << cy_buf->getFreeSize();
        return paContinue;
    }
    callback_time = clock();
    cy_buf->write(data, size);
    return paContinue;
}

int PaCallback(const void *input, void *output,
               long unsigned frame_count,
               const PaStreamCallbackTimeInfo *time_info,
               PaStreamCallbackFlags status_flags, void *user_data)
{
    BtRecorder *pa_src = reinterpret_cast<BtRecorder*>(user_data);

    int size = frame_count;
    void *ptr = const_cast<void *>(input);

    return pa_src->Callback((int16_t *)ptr, size);
}
