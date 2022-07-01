#ifndef KD_ONLINE_H
#define KD_ONLINE_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include <QDebug>

#include "bt_recorder.h"
#include "bt_captain.h" //For BtWord
#include "kd_model.h"
#include "kd_decodable.h"
#include "kd_online_ldecoder.h"
#include "bt_wav_writer.h"
#include "bt_state.h"

class KdOnline : public QObject
{
    Q_OBJECT
public:
    explicit KdOnline(BtState *state, QObject *parent = nullptr);
    ~KdOnline();

    void startDecode();

public slots:
    void init();

private:
    void processResult(QVector<BtWord> result);
    void writeWav(int len);

    void processLat(KdCompactLattice *clat, clock_t start);
    bool isSleep();
    bool isHalt();

    BtRecorder       *ab_src;
    QVector<BtWord>   c_result; // current_result
    KdOnlineStatus    status;
    BtCyclic         *cy_buf;
    BtCaptain        *cap;
    BtWavWriter      *wav_w;

    KdOnlineLDecoder *o_decoder;

    KdTransitionModel *t_model;
    KdAModel          *oa_model; //online accoustic model
    BtState           *st;
};

#endif // KD_ONLINE_H
