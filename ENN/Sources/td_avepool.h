#ifndef TDAVEPOOL_H
#define TDAVEPOOL_H

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "tiny_dnn/layers/partial_connected_layer.h"
#include "tiny_dnn/util/util.h"

// forward_propagation
inline void tiny_average_pooling_kernel(
        bool parallelize,
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        std::vector<tiny_dnn::tensor_t *> &out_data,
        const tiny_dnn::shape3d &out_dim,
        float_t scale_factor,
        std::vector<typename tiny_dnn::partial_connected_layer::wi_connections> &out2wi);

// back_propagation
inline void tiny_average_pooling_back_kernel(
        bool parallelize,
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        const std::vector<tiny_dnn::tensor_t *> &out_data,
        std::vector<tiny_dnn::tensor_t *> &out_grad,
        std::vector<tiny_dnn::tensor_t *> &in_grad,
        const tiny_dnn::shape3d &in_dim,
        float_t scale_factor,
        std::vector<typename tiny_dnn::partial_connected_layer::io_connections> &weight2io,
        std::vector<typename tiny_dnn::partial_connected_layer::wo_connections> &in2wo,
        std::vector<std::vector<size_t>> &bias2out);


class TdAvePool : public tiny_dnn::partial_connected_layer
{
public:
    TdAvePool(size_t in_width,
              size_t in_height,
              size_t in_channels,
              size_t pool_size_x,
              size_t pool_size_y,
              size_t stride_x,
              size_t stride_y,
              bool ceil_mode   = false,
              tiny_dnn::padding pad_type = tiny_dnn::padding::valid);

    std::vector<tiny_dnn::index3d<size_t>> in_shape() const override;

    std::vector<tiny_dnn::index3d<size_t>> out_shape() const override;

    std::string layer_type() const override;

    void forward_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
                             std::vector<tiny_dnn::tensor_t *> &out_data) override;

    void back_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
                  const std::vector<tiny_dnn::tensor_t *> &out_data,
                  std::vector<tiny_dnn::tensor_t *> &out_grad,
                  std::vector<tiny_dnn::tensor_t *> &in_grad) override;

    std::pair<size_t, size_t> pool_size() const;

private:
    size_t stride_x_;
    size_t stride_y_;
    size_t pool_size_x_;
    size_t pool_size_y_;
    tiny_dnn::padding pad_type_;
    bool ceil_mode_;
    tiny_dnn::shape3d in_;
    tiny_dnn::shape3d out_;
    tiny_dnn::shape3d w_;

    static size_t pool_out_dim(size_t in_size,
                               size_t pooling_size,
                               size_t stride);

    void init_connection(size_t pooling_size_x, size_t pooling_size_y);

    void connect_kernel(size_t pooling_size_x,
                        size_t pooling_size_y,
                        size_t x,
                        size_t y,
                        size_t inc);
};

#endif // TDAVEPOOL_H
