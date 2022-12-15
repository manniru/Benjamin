#include "ab_recorder.h"
#include <fcntl.h>
#include <unistd.h>

AbRecorder::AbRecorder(int sample_count, QObject *parent): QObject(parent)
{
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

    buf_size = sample_count;
    cy_buf = (int16_t *)malloc(buf_size * sizeof(int16_t));
    buf_index = 0;
    last_percent = 0;
}

AbRecorder::~AbRecorder()
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
    if( cy_buf )
    {
        free(cy_buf);
    }
}

void AbRecorder::openMic()
{
    PaError pa_err = Pa_OpenDefaultStream(&pa_stream, 1, 0, paInt16,
                                          BT_REC_RATE, 0, PaCallback, this);
    if( pa_err )
    {
        qDebug() << "PortAudio failed to open the default stream";
        pa_stream = NULL;
    }

    state = AB_STATUS_BREAK;
    PaError paerr = Pa_StartStream(pa_stream);
    if( paerr )
    {
        qDebug() << "Failed to open PortAudio stream";
    }
}

// This function deosn't need to be called
// Only use if you don't plan on using read function
void AbRecorder::reset()
{
    state = AB_STATUS_REC;
    buf_index = 0;
}

int AbRecorder::Callback(int16_t *data, int size)
{
    if( state==AB_STATUS_BREAK )
    {
        return paContinue;
    }

    int free_len = buf_size-buf_index;
    if( free_len<size )
    {
        for( int i=0 ; i<free_len ; i++ )
        {
            cy_buf[buf_index] = data[i];
            buf_index++;
        }
        qDebug() << "Recording is done";
        state = AB_STATUS_BREAK;
        emit finished();
        return paContinue;
    }

    for( int i=0 ; i<size ; i++ )
    {
        cy_buf[buf_index] = data[i];
        buf_index++;
    }

    int percent = (100*buf_index)/buf_size;
    if( last_percent!=percent )
    {
        last_percent = percent;
        emit updatePercent(last_percent);
    }

    return paContinue;
}

int PaCallback(const void *input, void *output,
               long unsigned frame_count,
               const PaStreamCallbackTimeInfo *time_info,
               PaStreamCallbackFlags status_flags, void *user_data)
{
    AbRecorder *pa_src = reinterpret_cast<AbRecorder*>(user_data);

    int size = frame_count;
    void *ptr = const_cast<void *>(input);

    return pa_src->Callback((int16_t *)ptr, size);
}
