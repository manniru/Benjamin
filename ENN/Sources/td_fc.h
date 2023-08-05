#ifndef TD_FC_H
#define TD_FC_H

#include <QObject>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "td_layer.h"
#include "tiny_dnn/core/kernels/fully_connected_op.h"
#include "tiny_dnn/core/kernels/fully_connected_grad_op.h"

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

    void forward_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
                        std::vector<tiny_dnn::tensor_t *> &out_data) override;

    void back_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
                      const std::vector<tiny_dnn::tensor_t *> &out_data,
                      std::vector<tiny_dnn::tensor_t *> &out_grad,
                      std::vector<tiny_dnn::tensor_t *> &in_grad) override;

    std::string layer_type() const override;

    protected:
    void set_params(const size_t in_size,
                    const size_t out_size, bool has_bias);

    void init_backend();

    private:
    /* The layer parameters */
    tiny_dnn::core::fully_params params_;

    /* forward op context */
    tiny_dnn::core::OpKernelContext fwd_ctx_;

    /* backward op context */
    tiny_dnn::core::OpKernelContext bwd_ctx_;

    /* Forward and backward ops */
    tiny_dnn::FullyConnectedOp     *kernel_fwd_;
    tiny_dnn::FullyConnectedGradOp *kernel_back_;
};

#endif // TD_FC_H
