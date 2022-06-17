#ifndef ENN_NETWORK_H
#define ENN_NETWORK_H

#include <QString>
#include <QDir>
#include <QtDebug>

#include <tiny_dnn/tiny_dnn.h>
#include <enn_dataset.h>

using namespace std;
using namespace tiny_dnn;

class EnnNetwork
{
public:
    EnnNetwork(QString word);
    ~EnnNetwork();

    void createEnn();

private:
    void epochLog();
    void minibatchLog();
    void shuffleTest(mt19937 *eng1, mt19937 *eng2);

    network<sequential> net;
    progress_display *disp;
    EnnDataset       *dataset;

    int n_minibatch;
    int n_train_epochs;
};

#endif // ENN_NETWORK_H
