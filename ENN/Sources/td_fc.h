#ifndef TD_FC_H
#define TD_FC_H

#include <QObject>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "tiny_dnn/core/params/fully_params.h"
#include "td_layer.h"
#include "tiny_dnn/util/product.h"

class TdFC : public TdLayer
{
public:
    TdFC(size_t in_dim,
         size_t out_dim,
         bool has_bias = true);

    size_t fan_in_size() const override;

    size_t fan_out_size() const override;

    std::vector<tiny_dnn::index3d<size_t>> in_shape() const override;

    std::vector<tiny_dnn::index3d<size_t>> out_shape() const override;

    void forward_propagation(const std::vector<tiny_dnn::tensor_t *> &in,
                        std::vector<tiny_dnn::tensor_t *> &out, int s_index, int e_index) override;

    void back_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
                      const std::vector<tiny_dnn::tensor_t *> &out_data,
                      std::vector<tiny_dnn::tensor_t *> &out_grad,
                      std::vector<tiny_dnn::tensor_t *> &in_grad, int s_index, int e_index) override;

    std::string layer_type() const override;

    protected:
    void set_params(const size_t in_size,
                    const size_t out_size, bool has_bias);

    private:

    template <typename Allocator>
    void avx_op_forward(
            const std::vector<std::vector<float, Allocator> > &in_data,
            const std::vector<float, Allocator> &W,
            const std::vector<float, Allocator> &bias,
            std::vector<std::vector<float, Allocator> > &out_data,
            const tiny_dnn::core::fully_params &params);
    template <typename Allocator>
    void avx_op_backward(
            const std::vector<std::vector<float, Allocator>> &prev_out,
            const std::vector<float, Allocator> &W,
            std::vector<std::vector<float, Allocator>> &dW,
            std::vector<std::vector<float, Allocator>> &db,
            std::vector<std::vector<float, Allocator>> &curr_delta,
            std::vector<std::vector<float, Allocator>> &prev_delta,
            const tiny_dnn::core::fully_params &params);
    void op_forward(const tiny_dnn::tensor_t &in_data,
                    const tiny_dnn::vec_t &W,
                    const tiny_dnn::vec_t &bias,
                    tiny_dnn::tensor_t &out_data,
                    const tiny_dnn::core::fully_params &params,
                    int s_index, int e_index);
    void op_backward(const tiny_dnn::tensor_t &prev_out,
                    const tiny_dnn::vec_t &W,
                    tiny_dnn::tensor_t &dW,
                    tiny_dnn::tensor_t &db,
                    tiny_dnn::tensor_t &curr_delta,
                    tiny_dnn::tensor_t &prev_delta,
                    const tiny_dnn::core::fully_params &params,
                    int s_index, int e_index);

    /* The layer parameters */
    tiny_dnn::core::fully_params params_;
};

#endif // TD_FC_H
