#ifndef TD_CONVOLUTION_H
#define TD_CONVOLUTION_H

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "tiny_dnn/core/params/conv_params.h"

#ifdef CNN_USE_AVX
#include "tiny_dnn/core/kernels/avx_kernel_common.h"
#endif

#include "td_layer.h"
#include "tiny_dnn/util/util.h"

class TdConvolution : public TdLayer
{
public:
    TdConvolution(size_t in_width,
                  size_t in_height,
                  size_t window_width,
                  size_t window_height,
                  size_t in_channels,
                  size_t out_channels,
                  tiny_dnn::padding pad_type   = tiny_dnn::padding::valid,
                  bool has_bias                = true,
                  size_t w_stride              = 1,
                  size_t h_stride              = 1,
                  size_t w_dilation            = 1,
                  size_t h_dilation            = 1);

    size_t fan_in_size() const override;

    size_t fan_out_size() const override;

    void forward_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
            std::vector<tiny_dnn::tensor_t *> &out_data, int s_index, int e_index) override;

    void back_propagation(
            const std::vector<tiny_dnn::tensor_t *> &in_data,
            const std::vector<tiny_dnn::tensor_t *> &out_data,
            std::vector<tiny_dnn::tensor_t *> &out_grad,
            std::vector<tiny_dnn::tensor_t *> &in_grad) override;

    void set_sample_count(size_t sample_count) override;

    std::vector<tiny_dnn::index3d<size_t>> in_shape() const override;

    std::vector<tiny_dnn::index3d<size_t>> out_shape() const override;

    std::string layer_type() const override;

    // TODO(edgar): is it really needed?
    std::string kernel_header() const override;

private:
    tiny_dnn::tensor_t *in_data_padded(
            const std::vector<tiny_dnn::tensor_t *> &in);

    void conv_set_params(
            const tiny_dnn::shape3d &in,
            size_t w_width,
            size_t w_height,
            size_t outc,
            tiny_dnn::padding ptype,
            bool has_bias,
            size_t w_stride,
            size_t h_stride,
            size_t w_dilation,
            size_t h_dilation,
            const tiny_dnn::core::connection_table &tbl =
            tiny_dnn::core::connection_table());

    template <typename Allocator>
    void avx_5x5_forward_kernel(
            const tiny_dnn::core::conv_params &params,
            const std::vector<float, Allocator> &in,
            const std::vector<float, Allocator> &W,
            const std::vector<float, Allocator> &bias,
            std::vector<float, Allocator> &a);

    void avx_op_forward(const tiny_dnn::tensor_t &in_data,
                        const tiny_dnn::vec_t &W,
                        const tiny_dnn::vec_t &bias,
                        tiny_dnn::tensor_t &out_data,
                        const tiny_dnn::core::conv_params &params);

    void op_forward(const tiny_dnn::tensor_t &in_data,
                    const tiny_dnn::vec_t &W,
                    const tiny_dnn::vec_t &bias,
                    tiny_dnn::tensor_t &out_data,
                    const tiny_dnn::core::conv_params &params,
                    int s_index, int e_index);

    template <typename tensor_t, typename vec_t>
    void op_backward(const tensor_t &prev_out,
                     const vec_t &W,
                     tensor_t &dW,
                     tensor_t &db,
                     tensor_t &curr_delta,
                     tensor_t &prev_delta,
                     const tiny_dnn::core::conv_params &params);

    void avx_op_backward(const tiny_dnn::tensor_t &prev_out,
                         const tiny_dnn::vec_t &W,
                         tiny_dnn::tensor_t &dW,
                         tiny_dnn::tensor_t &db,
                         tiny_dnn::tensor_t &curr_delta,
                         tiny_dnn::tensor_t &prev_delta,
                         const tiny_dnn::core::conv_params &params);

    template <typename Allocator>
    void avx_accumulate_db(const tiny_dnn::index3d<size_t> &out,
                           const std::vector<float, Allocator> &curr_delta,
                           std::vector<float, Allocator> &db);

    template <typename Allocator>
    void avx_accumulate_dw(const tiny_dnn::core::conv_params &params,
                           const std::vector<float, Allocator> &prev_out,
                           const std::vector<float, Allocator> &curr_delta,
                           std::vector<float, Allocator> &dW,
                           std::vector<float, Allocator> &db);

    template <typename Allocator>
    void avx_5x5_backward_kernel(
            const tiny_dnn::core::conv_params &params,
            const std::vector<float, Allocator> &prev_out,
            const std::vector<float, Allocator> &W,
            std::vector<float, Allocator> &dW,
            std::vector<float, Allocator> &db,
            std::vector<float, Allocator> &curr_delta,
            std::vector<float, Allocator> *prev_delta);

    size_t in_length(size_t in_length,
                     size_t window_size,
                     tiny_dnn::padding pad_type) const;

    static size_t conv_out_dim(size_t in_width,
                               size_t in_height,
                               size_t window_size,
                               size_t w_stride,
                               size_t h_stride,
                               size_t w_dilation,
                               size_t h_dilation,
                               tiny_dnn::padding pad_type);

    size_t conv_out_dim(size_t in_width,
                        size_t in_height,
                        size_t window_width,
                        size_t window_height,
                        size_t w_stride,
                        size_t h_stride,
                        size_t w_dilation,
                        size_t h_dilation,
                        tiny_dnn::padding pad_type) const;

private:
    /* The convolution parameters */
    tiny_dnn::core::conv_params params_;

    /* Padding operation */
    tiny_dnn::core::Conv2dPadding padding_op_;

    std::vector<tiny_dnn::tensor_t *> fwd_in_data_;
    std::vector<tiny_dnn::tensor_t *> bwd_in_data_;
    std::vector<tiny_dnn::tensor_t *> bwd_in_grad_;

    /* Buffer to store padded data */
    struct conv_layer_worker_specific_storage
    {
        tiny_dnn::tensor_t prev_out_padded_;
        tiny_dnn::tensor_t prev_delta_padded_;
    } cws_;
};

#endif // TD_CONVOLUTION_H
