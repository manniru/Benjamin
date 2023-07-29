#include "td_convolution.h"

TdConvolution::TdConvolution(size_t in_width, size_t in_height,
                             size_t window_width, size_t window_height,
                             size_t in_channels, size_t out_channels,
                             tiny_dnn::padding pad_type, bool has_bias,
                             size_t w_stride, size_t h_stride,
                             size_t w_dilation, size_t h_dilation)
    : TdLayer(tiny_dnn::std_input_order(has_bias),
{tiny_dnn::vector_type::data})
{
    conv_set_params(tiny_dnn::shape3d(in_width, in_height, in_channels),
                    window_width, window_height, out_channels,
                    pad_type, has_bias, w_stride,
                    h_stride, w_dilation, h_dilation,
                    tiny_dnn::core::connection_table());
    init_backend();
}

size_t TdConvolution::fan_in_size() const
{
    return params_.weight.width_ * params_.weight.height_ *
            params_.in.depth_;
}

size_t TdConvolution::fan_out_size() const
{
    return (params_.weight.width_ / params_.w_stride) *
            (params_.weight.height_ / params_.h_stride) *
            params_.out.depth_;
}

void TdConvolution::forward_propagation(
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        std::vector<tiny_dnn::tensor_t *> &out_data)
{
    // apply padding to the input tensor
    padding_op_.copy_and_pad_input(*in_data[0], cws_.prev_out_padded_);

    fwd_in_data_.resize(in_data.size());
    std::copy(in_data.begin(), in_data.end(), fwd_in_data_.begin());
    fwd_in_data_[0] = in_data_padded(in_data);

    // forward convolutional op context
    fwd_ctx_.set_in_out(fwd_in_data_, out_data);
    fwd_ctx_.setParallelize(parallelized);
    fwd_ctx_.setEngine(getEngine());

    // launch convolutional kernel
    kernel_fwd_->compute(fwd_ctx_);
}

void TdConvolution::back_propagation(
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        const std::vector<tiny_dnn::tensor_t *> &out_data,
        std::vector<tiny_dnn::tensor_t *> &out_grad,
        std::vector<tiny_dnn::tensor_t *> &in_grad)
{
    bwd_in_data_.resize(in_data.size());
    std::copy(in_data.begin(), in_data.end(), bwd_in_data_.begin());
    bwd_in_data_[0] = in_data_padded(in_data);

    bwd_in_grad_.resize(in_grad.size());
    std::copy(in_grad.begin(), in_grad.end(), bwd_in_grad_.begin());
    if( params_.pad_type==tiny_dnn::padding::same )
    {
        bwd_in_grad_[0] = &cws_.prev_delta_padded_;
    }

    bwd_ctx_.set_in_out(bwd_in_data_, out_data, out_grad,
                        bwd_in_grad_);
    bwd_ctx_.setParams(&params_);
    //    bwd_ctx_.setParallelize(layer::parallelize());
    bwd_ctx_.setEngine(getEngine());

    // launch convolutional kernel
    kernel_back_->compute(bwd_ctx_);

    // unpad deltas
    padding_op_.copy_and_unpad_delta(cws_.prev_delta_padded_,
                                     *in_grad[0]);
}

void TdConvolution::set_sample_count(size_t sample_count)
{
    TdLayer::set_sample_count(sample_count);
    cws_.prev_delta_padded_.resize(sample_count,
                                   tiny_dnn::vec_t(
                                       params_.in_padded.size(),
                                       float_t(0)));
}

std::vector<tiny_dnn::index3d<size_t> > TdConvolution::in_shape() const
{
    if( params_.has_bias )
    {
        return {params_.in, params_.weight,
                    tiny_dnn::index3d<size_t>(1, 1, params_.out.depth_)};
    }
    else
    {
        return {params_.in, params_.weight};
    }
}

std::vector<tiny_dnn::index3d<size_t> > TdConvolution::out_shape() const
{
    return {params_.out};
}

std::string TdConvolution::layer_type() const
{
    return std::string("conv");
}

std::string TdConvolution::kernel_header() const
{
    std::stringstream ss;
    ss << "#define MULTI\n";
    ss << "#define KERNEL_H " << params_.weight.height_ << "\n";
    ss << "#define KERNEL_W " << params_.weight.width_ << "\n";
    ss << "#define CHANNELS " << params_.weight.depth_ << "\n";
    ss << "#define STRIDE_H " << params_.h_stride << "\n";
    ss << "#define STRIDE_W " << params_.w_stride << "\n";
    ss << "#define DILATION_H " << params_.h_dilation << "\n";
    ss << "#define DILATION_W " << params_.w_dilation << "\n";
    ss << "#define APPLY_BIAS " << params_.has_bias << "\n";
    ss << "#define OUTPUT_Z " << params_.out.depth_ << "\n";
    // TODO(edgar): REVISE THIS
    ss << "#define ZPAR " << params_.out.depth_ << "\n";
    return ss.str();
}

tiny_dnn::tensor_t *TdConvolution::in_data_padded(
        const std::vector<tiny_dnn::tensor_t *> &in)
{
    if( params_.pad_type==tiny_dnn::padding::valid )
    {
        return in[0];
    }
    else
    {
        return &cws_.prev_out_padded_;
    }
}

void TdConvolution::conv_set_params(const tiny_dnn::shape3d &in,
                                    size_t w_width, size_t w_height, size_t outc,
                                    tiny_dnn::padding ptype, bool has_bias, size_t w_stride,
                                    size_t h_stride, size_t w_dilation, size_t h_dilation,
                                    const tiny_dnn::core::connection_table &tbl)
{
    params_.in = in;
    params_.in_padded =
            tiny_dnn::shape3d(in_length(in.width_, w_width, ptype),
                              in_length(in.height_, w_height, ptype),
                              in.depth_);
    params_.out = tiny_dnn::shape3d(
                conv_out_length(in.width_, w_width, w_stride,
                                w_dilation, ptype),
                conv_out_length(in.height_, w_height, h_stride,
                                h_dilation, ptype), outc);
    params_.weight     = tiny_dnn::shape3d(w_width, w_height,
                                           in.depth_ * outc);
    params_.has_bias   = has_bias;
    params_.pad_type   = ptype;
    params_.w_stride   = w_stride;
    params_.h_stride   = h_stride;
    params_.w_dilation = w_dilation;
    params_.h_dilation = h_dilation;
    params_.tbl        = tbl;

    // init padding buffer
    if (params_.pad_type == tiny_dnn::padding::same) {
        cws_.prev_delta_padded_.resize(
                    1, tiny_dnn::vec_t(params_.in_padded.size(),
                                       float_t(0)));
    }

    // set parameters to padding operation
    padding_op_ = tiny_dnn::core::Conv2dPadding(params_);
}

size_t TdConvolution::in_length(size_t in_length, size_t window_size,
                                tiny_dnn::padding pad_type) const
{
    if( pad_type==tiny_dnn::padding::same )
    {
        return in_length + window_size - 1;
    }
    else
    {
        return in_length;
    }
}

size_t TdConvolution::conv_out_dim(size_t in_width, size_t in_height,
                                   size_t window_size, size_t w_stride,
                                   size_t h_stride, size_t w_dilation,
                                   size_t h_dilation,
                                   tiny_dnn::padding pad_type)
{
    return conv_out_length(in_width, window_size, w_stride, w_dilation,
                           pad_type) *
            conv_out_length(in_height, window_size, h_stride, h_dilation,
                            pad_type);
}

size_t TdConvolution::conv_out_dim(size_t in_width, size_t in_height,
                                   size_t window_width,
                                   size_t window_height,
                                   size_t w_stride, size_t h_stride,
                                   size_t w_dilation, size_t h_dilation,
                                   tiny_dnn::padding pad_type) const
{
    return conv_out_length(in_width, window_width, w_stride,
                           w_dilation, pad_type) *
            conv_out_length(in_height, window_height, h_stride,
                            h_dilation, pad_type);
}

void TdConvolution::init_backend()
{
    tiny_dnn::core::OpKernelConstruction ctx =
            tiny_dnn::core::OpKernelConstruction(&params_);

    kernel_fwd_.reset(new tiny_dnn::Conv2dOp(ctx));
    kernel_back_.reset(new tiny_dnn::Conv2dGradOp(ctx));
}
