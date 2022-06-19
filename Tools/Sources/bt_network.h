#ifndef ENN_NETWORK_H
#define ENN_NETWORK_H
#include <QString>
#include <QDir>
#include <QDebug>
#include <tiny_dnn/tiny_dnn.h>

#include "backend.h"
#include "config.h"
#include "bt_cfb.h"

class BtNetwork
{
public:
    BtNetwork();
    ~BtNetwork();

    float predict(int id, float *data);

private:
    QVector<TdNetwork *> nets;
    QStringList word_list;
};

#endif // ENN_NETWORK_H
