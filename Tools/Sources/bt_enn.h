#ifndef BT_ENN_H
#define BT_ENN_H

// BT Ehsan Neural Network Feature Extractor

#include <QObject>
#include <QDebug>
#include <QFile>
#include <QImage>

#include "bt_captain.h" //For BtWord
#include "kd_model.h"
#include "kd_decodable.h"
#include "kd_online_ldecoder.h"
#include "bt_wav_writer.h"
#include "bt_state.h"

class BtEnn: public QObject
{
    Q_OBJECT
public:
    explicit BtEnn(QString dir_name, BtState *state,
                   QObject *parent = nullptr);
    ~BtEnn();

    void   init(QString dir);

private:
    void fillExist();
    void startDecode();
    void openWave(QString filename);
    void readWav(BtCyclic *out);
    void saveFeature(QString filename, BtCFB *cfb);
    void writeSample(QString filename, QVector<BtFrameBuf *> data);
    void saveImage(QString filename, QVector<BtFrameBuf *> data);
    void saveCSV(QString filename, QVector<BtFrameBuf *> data);
    void saveWave(QString filename);
    bool checkExist(QString path);
    void calcStat(QVector<BtFrameBuf *> data);
    void preProcess();
    bool sanityCheck(QString filename);

    BtCyclic         *cy_buf;
    KdOnlineStatus    status;
    KdOnlineLDecoder *o_decoder;
    KdTransitionModel *t_model;
    KdAModel         *oa_model; //online accoustic model
    BtWavWriter      *wav_w;
    QVector<BtWord>   last_r; //last_result
    QFile             wav_file;
    QStringList       file_list;
    QStringList       exist_list;
    QString           cat_dir; // category directory
    QString           enn_dir; // enn sample directory
    BtState           *st;

    double max_delta[3];
    double min_delta[3];
    double offset_delta = -5;
    double scale_delta = 19;
    int shit_counter = 0;
};

#endif // BT_ENN_H
