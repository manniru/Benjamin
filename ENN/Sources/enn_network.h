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

class EnnNetwork
{
public:
    EnnNetwork(QString word);
    ~EnnNetwork();

    bool  load();
    vec_t test(vec_t *data);
    void  train(float l_rate);
    void  benchmark();

    EnnDataset       *dataset;

private:
    void  save();
    void  epochLog();
    float calcLoss();
    void  createNNet();
    QString getAcc(std::vector<vec_t> &data,
                   std::vector<label_t> &label);
    void handleWrongs(float diff, QVector<int> &wrong_i,
                      QVector<float> &wrong_loss);

    network<sequential> net;
    progress_display *disp;
    adagrad optim;

    int n_minibatch;
    int n_train_epochs;
    int is_wrong;
};

#endif // ENN_NETWORK_H
