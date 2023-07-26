#ifndef TD_NODES_H
#define TD_NODES_H

#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <QObject>
#include <QDebug>

//#include "tiny_dnn/layers/layer.h"
#include "tiny_dnn/optimizers/optimizer.h"
#include "tiny_dnn/util/util.h"
#include "td_layer.h"

/** this class holds list of pointer of Node, and provides entry point of
 * forward / backward operations.
 * this is a computational unit of tiny-dnn (for example, convolution).
 * this can accept lvalue, rvalue and shared_ptr forms of node.
 * If given type is rvalue or shared_ptr, nodes create shared_ptr<node> to keep
 * given node alive. If given type is lvalue, tiny-dnn holds raw-pointer only
 * (to avoid double-free).
 *     sequential s;
 *     s.add(fc(100, 200));                          // rvalue, moved into nodes
 *     s.add(std::make_shared<fc>(200, 100));        // shared_ptr, shared by
 *nodes
 *     fc out(100, 10);
 *     softmax sft();
 *     s.add(out);                                   // lvalue, hold raw-pointer
 *only
 *
 * single-input, single-output feedforward network
 **/
class TdSequential : public QObject
{
    Q_OBJECT
public:
    explicit TdSequential(QObject *parent = nullptr);

    typedef std::vector<TdLayer *>::iterator iterator;
    typedef std::vector<TdLayer *>::const_iterator const_iterator;

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

    QVector<TdLayer *> nod;

private:
    std::vector<tiny_dnn::tensor_t> normalizeOut(
      const std::vector<const tiny_dnn::tensor_t *> &out);
    void connectHeadToTail();
};

#endif // TD_NODES_H
