#ifndef KD_ONLINE_H
#define KD_ONLINE_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>
#include <QDebug>

#include "bt_recorder.h"
#include "bt_captain.h" //For BtWord
#include "kd_online2_model.h"
#include "kd_online2_decodable.h"
#include "kd_online_ldecoder.h"

#define  BT_ONLINE_DECODER   KdOnlineLDecoder
#define  BT_ONLINE_DECODABLE KdOnline2Decodable
#define  BT_ONLINE_OPTS      KdOnlineLDecoderOpts
#define  BT_ONLINE_LAT       kaldi::CompactLattice

class KdOnline : public QObject
{
    Q_OBJECT
public:
    explicit KdOnline(QObject *parent = nullptr);
    ~KdOnline();

    void startDecode();

public slots:
    void init();

signals:
    void resultReady(QVector<BtWord> result);

private:
    void parseLexicon(QString filename);
    void execute(QVector<BtWord> result);

    void processLat(BT_ONLINE_LAT *clat, clock_t start);

    BtRecorder       *ab_src;
    QVector<QString>  history;
    QVector<QString>  lexicon;
    QVector<BtWord>   last_r; //last_result
    KdOnlineStatus    status;
    BtCyclic         *cy_buf;
};

#endif // KD_ONLINE_H
