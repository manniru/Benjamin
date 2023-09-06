#ifndef TD_LAYER_H
#define TD_LAYER_H

#include <algorithm>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <queue>
#include <sstream>
#include <string>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>
#include <QObject>
#include <QDebug>

#include "tiny_dnn/util/parallel_for.h"
#include "tiny_dnn/util/product.h"
#include "tiny_dnn/util/util.h"
#include "tiny_dnn/util/weight_init.h"

#include "tiny_dnn/optimizers/optimizer.h"

class TdEdge;

class TdLayer
{
public:
    explicit TdLayer(std::vector<tiny_dnn::vector_type> in_type);
    virtual ~TdLayer() = default;

    void setParallelize(bool parallelize);
    virtual std::string kernel_file() const;
    virtual std::string kernel_header() const;
    size_t inDataSize() const;
    size_t outDataSize() const;
    void setOutGrads(std::vector<tiny_dnn::tensor_t> &grad,
                     int s_index, int e_index);
    void setInData(const std::vector<tiny_dnn::tensor_t> &data, int batch_size, int offset);
    void setInData(tiny_dnn::vec_t &data);
    void setTrainable(bool tr);

    virtual std::pair<float_t, float_t> out_value_range() const;
    virtual std::vector<tiny_dnn::shape3d> in_shape() const = 0;
    virtual void set_in_shape(const tiny_dnn::shape3d &in_shape);
    virtual std::vector<tiny_dnn::shape3d> out_shape() const = 0;
    virtual std::string layer_type() const = 0;
    virtual size_t fan_in_size() const;
    virtual size_t fan_in_size(size_t) const;
    virtual size_t fan_out_size() const;
    virtual size_t fan_out_size(size_t) const;
    virtual void forward(int s_index, int e_index) = 0;
    virtual void back_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
                                  tiny_dnn::tensor_t *out_data,
                                  tiny_dnn::tensor_t *out_grad,
                                  std::vector<tiny_dnn::tensor_t *> &in_grad,
                                  int s_index, int e_index) = 0;
    void backward(int s_index, int e_index);
    void setup(bool reset_weight);
    void initWeight();
    void clearGrads(int batch_size);
    void updateWeight(tiny_dnn::optimizer *o, int batch_size);
    virtual void set_sample_count(size_t sample_count);

    std::vector<TdEdge *> in_edges;
    TdEdge *out_edges = 0;

    template <typename T, typename Func>
    inline void for_i(T size, Func f, size_t grainsize = 100);

    /** Flag indication whether the layer/node is initialized */
    bool initialized;
    /** Flag indicating whether the layer/node operations ara paralellized */
    bool parallelized;
    /** The number of input vectors/edges */
    size_t in_channels;
    /** Vector containing the type of data for inputs */
    std::vector<tiny_dnn::vector_type> in_type;
    /** Used in update_weight method. Kept as a member variable to reduce
     * frequent
     * memory allocation */
    tiny_dnn::vec_t weights_diff;
    /** Pointer to the function for weights initialization */
    tiny_dnn::weight_init::function *weight_init;
    /** Pointer to the function for biases initialization */
    tiny_dnn::weight_init::function *bias_init;
    /** Flag indicating whether the layer/node parameters are trainable */
    bool trainable;

private:
    std::vector<tiny_dnn::tensor_t *> fwd_in_data;
    std::vector<tiny_dnn::tensor_t *> bwd_in_data;
    std::vector<tiny_dnn::tensor_t *> bwd_in_grad;
    tiny_dnn::tensor_t *fwd_out_data;
    tiny_dnn::tensor_t *bwd_out_data;
    tiny_dnn::tensor_t *bwd_out_grad;
};

void td_connectLayer(TdLayer *head, TdLayer *tail);
void data_mismatch(TdLayer *layer, const tiny_dnn::vec_t &data);

class TdEdge
{
public:
    TdEdge(TdLayer *prev, tiny_dnn::shape3d shape,
         tiny_dnn::vector_type vtype)
        : shape_(shape),
          vtype_(vtype),
          data_({tiny_dnn::vec_t(shape.size())}),
          grad_({tiny_dnn::vec_t(shape.size())}),
          prev_(prev)
    {

    }

    void merge_grads(tiny_dnn::vec_t *dst)
    {
        assert(!grad_.empty());
        const auto &grad_head = grad_[0];
        size_t size = grad_head.size();
        dst->resize(size);
        float_t *pdst = &(*dst)[0];
        // dst = grad_[0]
        std::copy(grad_head.begin(), grad_head.end(), pdst);
        // @todo consider adding parallelism
        size_t sample_count = grad_.size();
        for( size_t sample=1 ; sample<sample_count ; ++sample )
        {
            // dst += grad_[sample]
            vectorize::reduce<float_t>(&grad_[sample][0], size, pdst);
        }
    }

    void clear_grads(int s_index, int e_index)
    {
//        size_t sample_count = grad_.size();
        for( int sample=s_index ; sample<e_index ; sample++ )
        {
            auto &g = grad_[sample];
            vectorize::fill(&g[0], g.size(), float_t{0});
        }
    }

    void add_next_node(TdLayer* next)
    {
        next_.push_back(next);
    }

    tiny_dnn::tensor_t grad_;
    tiny_dnn::tensor_t data_;
    tiny_dnn::shape3d shape_;
    tiny_dnn::vector_type vtype_;
    TdLayer *prev_;                // previous node, "producer" of this tensor
    std::vector<TdLayer *> next_;  // next nodes, "consumers" of this tensor
};

#endif // TD_LAYER_H
