#ifndef KD_ONLINE_H
#define KD_ONLINE_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include <QDebug>
#include "bt_channel_l.h"

#include "bt_state.h"

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
    void parseWords(QString filename);
    void execute(std::vector<int32_t> word, QVector<QString> *history);

    int kDeltaOrder = 2; // delta-delta derivative order
    int kTimeout = 500; // for the PortAudio (half second)
    int kSampleFreq = 16000; // fixed to 16KHz
    int kPaRingSize = 32768; // PortAudio's ring buffer size in bytes
    // Report interval for PortAudio buffer overflows in number of feat. batches
    int kPaReportInt = 4;
    int cmn_window = 600, min_cmn_window = 100;

    QVector<QString>  history;
    QVector<QString> lexicon;
};

#endif // KD_ONLINE_H
