#ifndef BT_CMVN_H
#define BT_CMVN_H

#include <QObject>
#include <QThread>

class BtCMVN : public QObject
{
    Q_OBJECT
public:
    explicit BtCMVN(QThread *thread, QObject *parent = nullptr);
    ~BtCMVN();

    void compute();

private:
    std::string cmvn_rspecifier;
    std::string feat_rspecifier;
    std::string feat_wspecifier;
    std::string utt2spk_rspecifier;
};


#endif // BT_CMVN_H
