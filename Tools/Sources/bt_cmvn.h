#ifndef BT_CMVN_H
#define BT_CMVN_H

#include <QObject>
#include <QVector>
#include "bt_config.h"
#include "bt_cfb.h"

#define BT_CMVN_WINDOW 600

// Ring buffer for CMVN Feature Buffer
typedef struct BtCMVNRing
{
    double data[BT_CMVN_WINDOW][BT_FEAT_SIZE]; // raw cepstrum
    int    head = 0;
    int    tail = 0; // cmvn + delta + delta-delta
    int    len  = 0;
}BtCMVNRing;

class BtCMVN
{
public:
    BtCMVN(BtCFB *feat);
    ~BtCMVN();

    void calc(uint frame);
    void resetSum();

private:
    void readGlobal();
    void updateStat();
    void addFrame(BtFrameBuf *buf);

    BtCFB *i_feature; //input feature (global buffer filled from outside)
    double global_state[BT_FEAT_SIZE+1];   // reflects the state before we saw this
    double f_sum[BT_FEAT_SIZE]; // feature sum
    double full_sum[BT_FEAT_SIZE]; // global + feature = always N = cmn_window
    BtCMVNRing feature_buf; // feature to calc mean from
};


#endif // BT_CMVN_H
