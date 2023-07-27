#ifndef TD_NODES_H
#define TD_NODES_H

#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <QDebug>

#include "tiny_dnn/optimizers/optimizer.h"
#include "tiny_dnn/util/util.h"
#include "td_layer.h"

class TdSequential
{
public:
    explicit TdSequential();

    void backward(const std::vector<tiny_dnn::tensor_t> &first);
    std::vector<tiny_dnn::tensor_t> forward(
            const std::vector<tiny_dnn::tensor_t> &first);
    void add(TdLayer *layer);
    void checkConnectivity();
    void updateWeights(tiny_dnn::optimizer *opt);
    void setup(bool reset_weight);
    void clearGrads();
    size_t inDataSize();
    size_t outDataSize() const;

    float_t targetValueMin(int out_channel = 0) const;
    float_t targetValueMax(int out_channel = 0) const;
    void label2vec(const tiny_dnn::label_t *t, size_t num,
                   std::vector<tiny_dnn::vec_t> &vec);
    void label2vec(const std::vector<tiny_dnn::label_t> &labels,
                   std::vector<tiny_dnn::vec_t> &vec);

    // transform indexing so that it's more suitable for per-layer operations
    // input:  [sample][channel][feature]
    // output: [channel][sample][feature]
    void reorderForLayerwiseProcessing(
        const std::vector<tiny_dnn::tensor_t> &input,
        std::vector<std::vector<const tiny_dnn::vec_t *>> &output);

    std::vector<TdLayer *> nod;

private:
    std::vector<tiny_dnn::tensor_t> normalizeOut(
      const std::vector<const tiny_dnn::tensor_t *> &out);
    void connectHeadToTail();
};

#endif // TD_NODES_H
