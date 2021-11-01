#ifndef BT_RECORDER_H
#define BT_RECORDER_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QThread>

#include "online/online-audio-source.h"
#include "matrix/kaldi-vector.h"
#include "bt_cyclic.h"
#include <portaudio.h>
#include <pa_ringbuffer.h>

class BtRecorder: public QObject
{
    Q_OBJECT
public:
    explicit BtRecorder(BtCyclic *buffer, QObject *parent = nullptr);

    // Implementation of the OnlineAudioSourceItf
    bool Read(kaldi::Vector<float> *data);

    // The real PortAudio callback delegates to this one
    int Callback(int16_t *data,
                 int size);

    ~BtRecorder();

public slots:
    void startStream();

private:
    BtCyclic *cy_buf;

    PaStream *pa_stream;
    bool pa_started_; // becomes "true" after "pa_stream_" is started
    uint32 report_interval_; // interval (in Read() calls) to report PA rb overflows
    uint32 nread_calls_; // number of Read() calls so far
    uint32 noverflows_; // number of the ringbuf overflows since the last report
    uint32 samples_lost_; // samples lost, due to PA ring buffer overflow
};

// The actual PortAudio callback - delegates to OnlinePaSource->Callback()
int PaCallback(const void *input, void *output,
               long unsigned frame_count,
               const PaStreamCallbackTimeInfo *time_info,
               PaStreamCallbackFlags status_flags,
               void *user_data);



#endif // BT_RECORDER_H
