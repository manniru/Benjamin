#ifndef KD_ONLINE2_H
#define KD_ONLINE2_H

#include "bt_config.h"

#ifdef BT_ONLINE2

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include <QDebug>
#include "bt_channel_l.h"
#include "kd_online2_gmm.h"
#include "bt_state.h"

class KdOnline2 : public QObject
{
    Q_OBJECT
public:
    explicit KdOnline2(QObject *parent = nullptr);
    ~KdOnline2();

    void startDecode();
    void processData(float *wav_data, int len);

private:
    void writeBarResult();
    void parseWords(QString filename);
    void execute(std::vector<int32_t> word, QVector<QString> *history);
    void printTime(clock_t start);

    void print(kaldi::CompactLattice *clat);

    int kDeltaOrder = 2; // delta-delta derivative order
    int kTimeout = 500; // for the PortAudio (half second)
    int kSampleFreq = 16000; // fixed to 16KHz
    int kPaRingSize = 32768; // PortAudio's ring buffer size in bytes
    // Report interval for PortAudio buffer overflows in number of feat. batches
    int kPaReportInt = 4;
    float acoustic_scale = 0.05;

    QVector<QString>  history;
    QVector<QString>  lexicon;
    KdOnline2Gmm     *g_decoder;
};

#endif

#endif // KD_ONLINE2_H
