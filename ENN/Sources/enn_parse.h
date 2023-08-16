#ifndef ENNPARSE_H
#define ENNPARSE_H

#include <QObject>
#include <QVector>
#include "td_network.h"

#define CONV_LAYER_STR      "Conv"
#define FC_LAYER_STR        "FC"
#define LEAKY_RELU_STR      "LeakyRelu"
#define AVERAGE_POOLING_STR "AvePool"
#define SOFTMAX_LAYER_STR   "SoftMax"

typedef struct EnnLayerSpec
{
    QString layer_name;
    QVector<int> specs;
}EnnLayerSpec;

class EnnParse : public QObject
{
    Q_OBJECT
public:
    explicit EnnParse(TdNetwork *n, QObject *parent = nullptr);

    void parseNetwork(QString network);
    void save(QString filename);
    void load(QString filename);

    TdNetwork *net;

private:
    EnnLayerSpec parseLayer(QString data);
    void addToNetwork(EnnLayerSpec layer);

    QString    net_str;
};

#endif // ENNPARSE_H
