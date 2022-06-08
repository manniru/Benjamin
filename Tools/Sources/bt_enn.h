#ifndef BT_ENN_H
#define BT_ENN_H

// BT Ehsan Neural Network Feature Extractor

#include <QObject>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QImageWriter>

#include "bt_captain.h" //For BtWord
#include "kd_model.h"
#include "kd_decodable.h"
#include "kd_online_ldecoder.h"

class BtEnn: public QObject
{
    Q_OBJECT
public:
    explicit BtEnn(QString dir_name, QObject *parent = nullptr);
    ~BtEnn();

private:
    void   init();
    void   startDecode();
    void   openWave(QString filename);
    int    readWav(int count, BtCyclic *out);
    void   saveFeature(QString filename, BtCFB *cfb);
    void saveImage(QString filename, QVector<BtFrameBuf *> data);
    void saveCSV(QString filename, QVector<BtFrameBuf *> data);

    BtCyclic         *cy_buf;
    QVector<BtWord>   last_r; //last_result
    KdOnlineStatus    status;
    KdOnlineLDecoder *o_decoder;
    kaldi::TransitionModel *t_model;
    KdAModel         *oa_model; //online accoustic model
    QFile             wav_file;
    QStringList       file_list;

    double max_delta[3];
    double min_delta[3];
    double offset_delta[3] = {30, 10, 5};
    double scale_delta[3] = {4, 9, 20};
};

#endif // BT_ENN_H
