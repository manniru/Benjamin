#ifndef KD_ONLINE_H
#define KD_ONLINE_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include <QDebug>
#include "bt_channel_l.h"

#include "bt_state.h"
#include "bt_recorder.h"
#include "bt_encoder.h"

class KdOnline : public QObject
{
    Q_OBJECT
public:
    explicit KdOnline(QObject *parent = nullptr);
    ~KdOnline();

    void startDecode();

public slots:
    void init();

private:
    void writeBarResult();

    // Up to delta-delta derivative features are calculated (unless LDA is used)
    int kDeltaOrder = 2;
    // Time out interval for the PortAudio source
    int kTimeout = 500; // half second
    // Input sampling frequency is fixed to 16KHz
    int kSampleFreq = 16000;
    // PortAudio's internal ring buffer size in bytes
    int kPaRingSize = 32768;
    // Report interval for PortAudio buffer overflows in number of feat. batches
    int kPaReportInt = 4;
    int cmn_window = 600, min_cmn_window = 100;
    int right_context = 4, left_context = 4;

    QVector<QString>  history;
};

#endif // KD_ONLINE_H
