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
    void processResult(QVector<BtWord> result);
    void writeWav(BtCyclic *buf, int len);
    void writeWavHeader(QFile *file, int len);

    void processLat(KdCompactLattice *clat, clock_t start);
    bool isSleep();

    BtRecorder       *ab_src;
    QVector<BtWord>   last_r; //last_result
    KdOnlineStatus    status;
    BtCyclic         *cy_buf;
    BtCaptain        *cap;

    KdOnlineLDecoder *o_decoder;

    kaldi::TransitionModel *t_model;
    KdAModel               *oa_model; //online accoustic model
};

#endif // KD_ONLINE_H
