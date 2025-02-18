#ifndef ENN_NETWORK_H
#define ENN_NETWORK_H

#include <QString>
#include <QDir>
#include <QtDebug>
#include <QObject>
#include "td_network.h"
#include "enn_parse.h"
#include <tiny_dnn/util/util.h>
#ifdef ENN_IMAGE_DATASET
#include "enn_dataset_image.h"
#else
#include "enn_dataset.h"
#endif

#define ENN_NETWORK_NORMAL      0
#define ENN_NETWORK_NEGATIVE    1 // train loss is increasing
#define ENN_NETWORK_LOCKED      2 // network loss exploded

#define ENN_EXPLODE_LOSS        500
#define ENN_BATCH_SIZE          64

using namespace tiny_dnn;

typedef struct EnnResult
{
    QString msg;

    int tot_t = 0; //total true
    int det_t = 0; //corrrect detected true
    int tot_f = 0; //total false
    int det_f = 0; //corrrect detected false
} EnnResult;

class EnnNetwork : public QObject
{
    Q_OBJECT
public:
    explicit EnnNetwork(QString word, int id, QObject *parent = nullptr);
    ~EnnNetwork();

    bool  load();
    vec_t test(vec_t *data);
    void  train(float l_rate);
    void  benchmark();

    EnnDataset *dataset;
    float       last_loss;

private slots:
    void  epochLog();

private:
    void  save();
    float calcLoss();
    void  createNNet();
    EnnResult getAccuracy(std::vector<vec_t> &data,
                   std::vector<int> &label);
    void handleWrongs(float diff, QVector<int> &wrong_i,
                      QVector<float> &wrong_loss);
    float_t mse_f(vec_t &y, int &o);

    TdNetwork *net;
    EnnParse *parser;

    int n_train_epochs;
    int net_state; // network state
    bool need_train;
    clock_t start_time;
};

#endif // ENN_NETWORK_H
