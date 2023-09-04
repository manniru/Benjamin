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

#include "td_nodes.h"
#include "tiny_dnn/util/util.h"
#include "td_fc.h"
#include "td_convolution.h"
#include "td_avepool.h"
#include "td_leaky_relu.h"
#include "td_softmax.h"
#include "td_layer.h"
#include "td_worker.h"

class TdNetwork : public QObject
{
    Q_OBJECT
public:
    explicit TdNetwork(int bs, QObject *parent = nullptr);

    void initWeightBias();

    void stopOngoingTraining();
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
    void fit(std::vector<tiny_dnn::tensor_t> &inputs,
             std::vector<tiny_dnn::tensor_t> &desired_outputs,
             int epoch, bool reset_weights,
             std::vector<tiny_dnn::tensor_t> &t_cost);
    void normalizeTensor(const std::vector<tiny_dnn::tensor_t> &inputs,
                         std::vector<tiny_dnn::tensor_t> &normalized);
    void normalizeTensor(const std::vector<tiny_dnn::vec_t> &inputs,
                         std::vector<tiny_dnn::tensor_t> &normalized);
    void normalizeTensor(const std::vector<tiny_dnn::label_t> &inputs,
                          std::vector<tiny_dnn::tensor_t> &normalized);
    TdNetwork* addFC(int in_dim, int out_dim);
    TdNetwork* addLeakyRelu();
    TdNetwork* addConv(int in_width, int in_height, int window_width,
                 int window_height, int in_channels,
                 int out_channels);
    TdNetwork* addAvePool(int in_width, int in_height, int in_channels,
                    int pool_size_x, int pool_size_y, int stride_x,
                    int stride_y);
    TdNetwork* addSoftMax();

    tiny_dnn::vec_t predict(tiny_dnn::vec_t &first);
    void setBatchSize(int bs);

    std::vector<TdLayer *>  nod;
    TdWorker *worker;

signals:
    void trainFinished();
    void OnEpochEnumerate();

public slots:
    void epochEnumerate();

private:
    void checkTargetCostMatrix(const std::vector<tiny_dnn::tensor_t> &t,
                               const std::vector<tiny_dnn::tensor_t> &t_cost);
    void checkTargetCostElement(const tiny_dnn::vec_t &t,
                                const tiny_dnn::vec_t &t_cost);
    void checkTargetCostElement(const tiny_dnn::tensor_t &t,
                                const tiny_dnn::tensor_t &t_cost);
    void label2vec(const tiny_dnn::label_t *t, size_t num,
                                std::vector<tiny_dnn::vec_t> &vec);
    void setup(bool reset_weight);

    int batch_size;
};

#endif // TD_NETWORK_H
