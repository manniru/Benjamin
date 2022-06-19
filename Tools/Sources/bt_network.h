#ifndef BT_NETWORK_H
#define BT_NETWORK_H
#include <QString>
#include <QDir>
#include <QDebug>
#include <tiny_dnn/tiny_dnn.h>

#include "backend.h"
#include "config.h"
#include "bt_cfb.h"
#include "bt_wav_writer.h"

class BtNetwork
{
public:
    BtNetwork();
    ~BtNetwork();

    float getConf(int start, int len, int id);
    BtCFB       *cfb;
    BtWavWriter *wav_w;

private:
    float predict(int id);
    void  calcStat(int start, int len);
    void  saveWave(int start, int len, QString word);

    QVector<TdNetwork *> nets;
    QStringList word_list;
    float  data_buf[BT_ENN_SIZE*BT_ENN_SIZE*3];

    double offset_delta = -5;
    double scale_delta = 19;
};

#endif // BT_NETWORK_H
