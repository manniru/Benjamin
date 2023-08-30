#ifndef TD_NETWORK_H
#define TD_NETWORK_H

#include <QObject>
#ifndef CNN_NO_SERIALIZATION
#include <fstream>
#endif
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <QString>
#include <QDebug>
#include <type_traits>

//#include "tiny_dnn/nodes.h"
#include "td_nodes.h"
#include "tiny_dnn/lossfunctions/loss_function.h"
#include "tiny_dnn/optimizers/optimizer.h"
#include "tiny_dnn/util/util.h"
#include "td_fc.h"
#include "td_convolution.h"
#include "td_avepool.h"
#include "td_leaky_relu.h"
#include "td_softmax.h"
#include "td_layer.h"

class TdNetwork : public QObject
{
    Q_OBJECT
public:
    explicit TdNetwork(QString name = "", QObject *parent = nullptr);

    void initWeightBias();
    tiny_dnn::vec_t predict(const tiny_dnn::vec_t &in);
    void updateWeights(tiny_dnn::optimizer *opt);
    float_t predictMaxValue(const tiny_dnn::vec_t &in);
    tiny_dnn::label_t predictLabel(const tiny_dnn::vec_t &in);

    void setNetPhase(tiny_dnn::net_phase phase);
    void stopOngoingTraining();
    std::vector<tiny_dnn::vec_t> test(
            const std::vector<tiny_dnn::vec_t> &in);
    float_t getLoss(const std::vector<tiny_dnn::vec_t> &in,
                     const std::vector<tiny_dnn::label_t> &t);
    float_t getLoss(const std::vector<tiny_dnn::vec_t> &in,
                     const std::vector<tiny_dnn::vec_t> &t);
    template <typename T>
    float_t getLoss(const std::vector<T> &in,
                    const std::vector<tiny_dnn::tensor_t> &t);
    size_t layerSize();
    size_t depth();
    size_t outDataSize();
    size_t inDataSize();
    void load(const std::string &filename);
    void save(const std::string &filename) const;
    template <typename OutputArchive>
    void toArchive(OutputArchive &ar) const;
    template <typename InputArchive>
    void fromArchive(InputArchive &ar);
    bool fit(tiny_dnn::adagrad &optimizer,
             const std::vector<tiny_dnn::tensor_t> &inputs,
             const std::vector<tiny_dnn::tensor_t> &desired_outputs,
             size_t batch_size,
             int epoch,
             const bool reset_weights            = false,
             const std::vector<tiny_dnn::tensor_t> &t_cost =
                   std::vector<tiny_dnn::tensor_t>());
    void normalizeTensor(const std::vector<tiny_dnn::tensor_t> &inputs,
                         std::vector<tiny_dnn::tensor_t> &normalized);
    void normalizeTensor(const std::vector<tiny_dnn::vec_t> &inputs,
                         std::vector<tiny_dnn::tensor_t> &normalized);
    void normalizeTensor(const std::vector<tiny_dnn::label_t> &inputs,
                          std::vector<tiny_dnn::tensor_t> &normalized);
    float_t fprop_max(const tiny_dnn::vec_t &in);
    tiny_dnn::label_t fprop_max_index(const tiny_dnn::vec_t &in);
    TdNetwork* addFC(int in_dim, int out_dim);
    TdNetwork* addLeakyRelu();
    TdNetwork* addConv(int in_width, int in_height, int window_width,
                 int window_height, int in_channels,
                 int out_channels);
    TdNetwork* addAvePool(int in_width, int in_height, int in_channels,
                    int pool_size_x, int pool_size_y, int stride_x,
                    int stride_y);
    TdNetwork* addSoftMax();

    QString net_name;
    int     stop_training;
    std::vector<TdLayer *> nod;

signals:
    void onBatchEnumerate();
    void OnEpochEnumerate();

private:
    void trainMiniBatch(tiny_dnn::adagrad &optimizer,
                    const tiny_dnn::tensor_t *in,
                    const tiny_dnn::tensor_t *t, int size,
                    const tiny_dnn::tensor_t *t_cost);
    void checkTargetCostMatrix(const std::vector<tiny_dnn::tensor_t> &t,
                               const std::vector<tiny_dnn::tensor_t> &t_cost);
    void checkTargetCostElement(const tiny_dnn::vec_t &t,
                                const tiny_dnn::vec_t &t_cost);
    void checkTargetCostElement(const tiny_dnn::tensor_t &t,
                                const tiny_dnn::tensor_t &t_cost);
    void label2vec(const tiny_dnn::label_t *t, size_t num,
                                std::vector<tiny_dnn::vec_t> &vec);

    void backward(const std::vector<tiny_dnn::tensor_t> &first);
    std::vector<tiny_dnn::tensor_t> forward(
            const std::vector<tiny_dnn::tensor_t> &first);
    tiny_dnn::vec_t forward(tiny_dnn::vec_t &first);
    void setup(bool reset_weight);
    void clearGrads();
};

#endif // TD_NETWORK_H
