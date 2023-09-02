#ifndef TDAVEPOOL_H
#define TDAVEPOOL_H

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "td_layer.h"
#include "tiny_dnn/util/util.h"

class TdAvePool : public TdLayer
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
    size_t param_size() const;
    size_t fan_in_size() const override;
    size_t fan_out_size() const override;
    std::string layer_type() const override;
    void connect_weight(size_t input_index, size_t output_index,
                        size_t weight_index);
    void connect_bias(size_t bias_index, size_t output_index);
    void forward_propagation(
            const std::vector<tiny_dnn::tensor_t *> &in_data,
            std::vector<tiny_dnn::tensor_t *> &out_data, int s_index,
            int e_index) override;
    void back_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
                  const std::vector<tiny_dnn::tensor_t *> &out_data,
                  std::vector<tiny_dnn::tensor_t *> &out_grad,
                  std::vector<tiny_dnn::tensor_t *> &in_grad,
                  int s_index, int e_index) override;
    std::pair<size_t, size_t> pool_size() const;

    typedef std::vector<std::pair<size_t, size_t>> io_connections;
    typedef std::vector<std::pair<size_t, size_t>> wi_connections;
    typedef std::vector<std::pair<size_t, size_t>> wo_connections;

private:
    static size_t pool_out_dim(size_t in_size,
                               size_t pooling_size,
                               size_t stride);

    void init_connection(size_t pooling_size_x, size_t pooling_size_y);

    void connect_kernel(size_t pooling_size_x,
                        size_t pooling_size_y,
                        size_t x,
                        size_t y,
                        size_t inc);

    size_t stride_x_;
    size_t stride_y_;
    size_t pool_size_x_;
    size_t pool_size_y_;
    tiny_dnn::padding pad_type_;
    bool ceil_mode_;
    tiny_dnn::shape3d in_;
    tiny_dnn::shape3d out_;
    tiny_dnn::shape3d w_;

    std::vector<io_connections> weight2io_;  // weight_id -> [(in_id, out_id)]
    std::vector<wi_connections> out2wi_;     // out_id -> [(weight_id, in_id)]
    std::vector<wo_connections> in2wo_;      // in_id -> [(weight_id, out_id)]
    std::vector<std::vector<size_t>> bias2out_;
    std::vector<size_t> out2bias_;
    float_t scale_factor_;
};

// forward_propagation
inline void tiny_average_pooling_kernel(bool parallelize,
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        std::vector<tiny_dnn::tensor_t *> &out_data,
        const tiny_dnn::shape3d &out_dim,
        float_t scale_factor,
        std::vector<typename TdAvePool::wi_connections> &out2wi,
        int s_index, int e_index);

// back_propagation
inline void tiny_average_pooling_back_kernel(bool parallelize,
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        const std::vector<tiny_dnn::tensor_t *> &out_data,
        std::vector<tiny_dnn::tensor_t *> &out_grad,
        std::vector<tiny_dnn::tensor_t *> &in_grad,
        const tiny_dnn::shape3d &in_dim,
        float_t scale_factor,
        std::vector<typename TdAvePool::io_connections> &weight2io,
        std::vector<typename TdAvePool::wo_connections> &in2wo,
        std::vector<std::vector<size_t>> &bias2out,
        int s_index, int e_index);

#endif // TDAVEPOOL_H
