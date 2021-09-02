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


private:

};


#endif // BT_CMVN_H
