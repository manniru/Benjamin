#ifndef BT_CFB_H
#define BT_CFB_H

// Cyclic Frame Buffer
// Use to preserve cepstrum features in a
// cyclic buffer

#include <QObject>
#include <QVector>

#define BT_FEAT_SIZE   13    // number of cepstrum feature size
#define BT_FB_SIZE     10000 // number of frame in cyclic buf
#define BT_DELTA_ORDER 2
#define BT_DELTA_SIZE  (BT_DELTA_ORDER+1)*BT_FEAT_SIZE

typedef struct BtFrameBuf
{
    double  ceps [BT_FEAT_SIZE]; // raw cepstrum
    double  cmvn [BT_FEAT_SIZE]; // applied cmvn
    double  delta[BT_DELTA_SIZE]; // cmvn + delta + delta-delta
    bool    have_cmvn = false;
}BtFrameBuf;

class BtCFB : public QObject
{
    Q_OBJECT
public:
    explicit BtCFB(int size = BT_FB_SIZE, QObject *parent = nullptr);
    BtFrameBuf* get(uint frame);

private:
    BtFrameBuf *data;
    int len;
};

#endif // BT_CFB_H
