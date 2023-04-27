#ifndef BT_NETWORK_H
#define BT_NETWORK_H
#include <QString>
#include <QDir>
#include <QDebug>
#include <QImage>
#include <tiny_dnn/tiny_dnn.h>

#include "backend.h"
#include "config.h"
#include "bt_cfb.h"
#include "bt_wav_writer.h"
#include "bt_state.h"

class BtNetwork
{
public:
    BtNetwork(BtState *state);
    ~BtNetwork();

    float getConf(int start, int len, int id);
    void  fillImage(int start, int len);
    void  fillRaw(int start, int len);
    BtCFB *cfb;
    QImage img_s;

private:
    float predict(int id);
    void  calcStat(int start, int len);
    void  saveWave(int start, int len, QString word);

    QVector<TdNetwork *> nets;
    QStringList word_list;
#ifdef ENN_IMAGE_DATASET
    float  data_buf[BT_ENN_SIZE*BT_ENN_SIZE*3];
#else
    float  data_buf[BT_ENN_SIZE*BT_ENN_SIZE];
#endif

    double offset_delta = -5;
    double scale_delta = 19;
};

#endif // BT_NETWORK_H
