#ifndef AB_RECORDER_H
#define AB_RECORDER_H

#include <QObject>
#include <QDebug>
#include <QThread>

#include <portaudio.h>

#include "config.h"

#define BT_CON_TIMER     1000
#define BT_INV_TIME     -1

class AbRecorder: public QObject
{
    Q_OBJECT
public:
    explicit AbRecorder(int sample_count, QObject *parent = nullptr);

    // The real PortAudio callback delegates to this one
    int Callback(int16_t *data, int size);

    ~AbRecorder();
    int16_t *cy_buf;
    int buf_size;
    int buf_index;

public slots:
    void reset();

signals:
    void finished();
    void updatePercent(int percent);

private:
    void openMic();

    PaStream *pa_stream;
    int last_percent;
    int state;
};

// The actual PortAudio callback - delegates to OnlinePaSource->Callback()
int PaCallback(const void *input, void *output,
               long unsigned frame_count,
               const PaStreamCallbackTimeInfo *time_info,
               PaStreamCallbackFlags status_flags,
               void *user_data);



#endif // AB_RECORDER_H
