#ifndef ENN_NETWORK_H
#define ENN_NETWORK_H

#include <QString>
#include <QDir>
#include <QtDebug>

#include <tiny_dnn/tiny_dnn.h>
#ifdef ENN_IMAGE_DATASET
#include "enn_dataset_image.h"
#else
#include "enn_dataset.h"
#endif

using namespace tiny_dnn;

typedef struct EnnResult
{
    QString msg;

    int tot_t = 0; //total true
    int det_t = 0; //corrrect detected true
    int tot_f = 0; //total false
    int det_f = 0; //corrrect detected false
} EnnResult;

class EnnNetwork
{
public:
    EnnNetwork(QString word, int id);
    ~EnnNetwork();

    bool  load();
    vec_t test(vec_t *data);
    void  train(float l_rate);
    void  benchmark();

    EnnDataset *dataset;
    float       last_loss;

private:
    void  save();
    void  epochLog();
    float calcLoss();
    void  createNNet();
    EnnResult getAcc(std::vector<vec_t> &data,
                   std::vector<label_t> &label);
    void handleWrongs(float diff, QVector<int> &wrong_i,
                      QVector<float> &wrong_loss);

    network net;
    adagrad optim;

    int n_minibatch;
    int n_train_epochs;
    int is_wrong;
    bool need_train;
};

#endif // ENN_NETWORK_H
