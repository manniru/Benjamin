#ifndef BT_TEST_H
#define BT_TEST_H

// BT Test Mode
// Use wave file instead of online
// recorder

#include <QObject>
#include <QDebug>
#include <QFile>

#include "bt_captain.h" //For BtWord
#include "kd_online2_model.h"
#include "kd_decodable.h"
#include "kd_online_ldecoder.h"

class BtTest: public QObject
{
    Q_OBJECT
public:
    explicit BtTest(QString filename, QObject *parent = nullptr);
    ~BtTest();

private:
    void   init();
    void   startDecode();
    void   openWave(QString filename);
    int    readWav(int count, BtCyclic *out);

    BtCyclic         *cy_buf;
    QVector<BtWord>   last_r; //last_result
    KdOnlineStatus    status;
    KdOnlineLDecoder *o_decoder;
    KdOnline2Model   *o2_model; //gaussain online 2 model
    QFile            *wav_file;
};

#endif // BT_TEST_H
