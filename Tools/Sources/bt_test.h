#ifndef BT_TEST_H
#define BT_TEST_H

// BT Test Mode
// Use wave file instead of online
// recorder

#include <QObject>
#include <QDebug>
#include <QFile>

#include "bt_captain.h" //For BtWord
#include "kd_model.h"
#include "kd_decodable.h"
#include "kd_online_ldecoder.h"

class BtTest: public QObject
{
    Q_OBJECT
public:
    explicit BtTest(QString dir_name, QObject *parent = nullptr);
    ~BtTest();

private:
    void   init();
    void   startDecode();
    void   openWave(QString filename);
    int    readWav(int count, BtCyclic *out);
    float  getConf(BtWord word, QString prefix);
    void   saveWave(int start, int len, QString fname);
    void saveImage(QString fname);
    bool checkExist(QString path);

    BtCyclic         *cy_buf;
    QVector<BtWord>   last_r; //last_result
    KdOnlineStatus    status;
    KdOnlineLDecoder *o_decoder;
    kaldi::TransitionModel *t_model;
    KdAModel         *oa_model; //online accoustic model
    QFile             wav_file;
    QStringList       file_list;
    BtNetwork        *net;
};

#endif // BT_TEST_H
