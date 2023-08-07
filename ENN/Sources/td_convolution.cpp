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

void TdConvolution::op_forward(const tiny_dnn::tensor_t &in_data,
                               const tiny_dnn::vec_t &W,
                               const tiny_dnn::vec_t &bias,
                               tiny_dnn::tensor_t &out_data,
                               const tiny_dnn::core::conv_params &params)
{
    int parallelized = true;
    tiny_dnn::for_(parallelized, 0u, in_data.size(),
                   [&](const tiny_dnn::blocked_range &r)
    {
        size_t out_area    = params.out.area();
        size_t iw          = params.in_padded.width_;
        size_t id          = params.in.depth_;
        size_t ow          = params.out.width_;
        size_t oh          = params.out.height_;
        size_t od          = params.out.depth_;
        size_t kw          = params.weight.width_;
        size_t kh          = params.weight.height_;
        size_t w_dilation  = params.w_dilation;
        size_t h_dilation  = params.h_dilation;
        size_t elem_stride = params.w_stride;
        size_t line_stride = iw * params.h_stride;
        for (size_t sample = r.begin(); sample < r.end(); sample++)
        {
            const tiny_dnn::vec_t &in = in_data[sample];
            tiny_dnn::vec_t &a        = out_data[sample];
            for (size_t o = 0; o < od; o++)
            {
                float_t *pa = &a[params.out.get_index(0, 0, o)];
                for (size_t inc = 0; inc < id; inc++)
                {
                    if (!params.tbl.is_connected(o, inc))
                    {
                        continue;
                    }
                    size_t idx;
                    idx = params.weight.get_index(0, 0, id * o + inc);
                    const float_t *pw  = &W[idx];
                    idx = params.in_padded.get_index(0, 0, inc);
                    const float_t *pin = &in[idx];
                    float_t *pout      = pa;
                    for (size_t y = 0; y < oh; y++)
                    {
                        const float_t *pin_line = pin;
                        for (size_t x = 0; x < ow; x++)
                        {
                            const float_t *pin_element = pin_line;
                            const float_t *pw_element  = pw;
                            float_t sum{0};
                            // should be optimized for small kernel(3x3,5x5)
                            for (size_t wy = 0; wy < kh; wy++)
                            {    // NOLINT
                                for (size_t wx = 0; wx < kw; wx++)
                                {  // NOLINT
                                    sum += pw_element[wx] *
                                            pin_element[wx * w_dilation];
                                }
                                pw_element += kw;
                                pin_element += iw * h_dilation;
                            }
                            pout[x] += sum;
                            pin_line += elem_stride;
                        }
                        pout += ow;
                        pin += line_stride;
                    }
                }
                if (params.has_bias)
                {
                    vectorize::add(bias[o], out_area, pa);
                }
            }
        }
    }, 0u);
}

template <typename tensor_t, typename vec_t>
void TdConvolution::op_backward(const tensor_t &prev_out,
                                const vec_t &W,
                                tensor_t &dW,
                                tensor_t &db,
                                tensor_t &curr_delta,
                                tensor_t &prev_delta,
                                const tiny_dnn::core::conv_params &params)
{
    typedef typename vec_t::value_type float_t;

    int parallelized = true;
    tiny_dnn::for_i(parallelized, prev_out.size(), [&](size_t sample)
    {
        // propagate delta to previous layer
        for (size_t inc = 0; inc < params.in.depth_; inc++)
        {
            for (size_t outc = 0; outc < params.out.depth_; outc++)
            {
                if (!params.tbl.is_connected(outc, inc))
                {
                    continue;
                }

                size_t idx        = 0;
                idx               = params.in.depth_ * outc + inc;
                idx               = params.weight.get_index(0, 0, idx);
                const float_t *pw = &W[idx];

                idx = params.out.get_index(0, 0, outc);
                const float_t *pdelta_src = &curr_delta[sample][idx];

                idx = params.in_padded.get_index(0, 0, inc);
                // float_t* pdelta_dst = &(*prev_delta)[sample][idx];
                float_t *pdelta_dst = &prev_delta[sample][idx];

                for (size_t y = 0; y < params.out.height_; y++)
                {
                    for (size_t x = 0; x < params.out.width_; x++)
                    {
                        const float_t *ppw = pw;

                        idx = y * params.out.width_ + x;
                        const float_t ppdelta_src = pdelta_src[idx];

                        float_t *ppdelta_dst = pdelta_dst + y *
                                params.h_stride *
                                params.in_padded.width_ + x *
                                params.w_stride;

                        for (size_t wy = 0; wy < params.weight.height_;
                             wy++)
                        {   // NOLINT
                            for (size_t wx = 0;
                                 wx < params.weight.width_; wx++)
                            {  // NOLINT
                                idx = wy * params.in_padded.width_ + wx;
                                ppdelta_dst[idx] += *ppw++ * ppdelta_src;
                            }
                        }
                    }
                }
            }
        }

        // accumulate dw
        for (size_t inc = 0; inc < params.in.depth_; inc++)
        {
            for (size_t outc = 0; outc < params.out.depth_; outc++)
            {
                if (!params.tbl.is_connected(outc, inc))
                {
                    continue;
                }

                for (size_t wy = 0; wy < params.weight.height_; wy++)
                {
                    for (size_t wx = 0; wx < params.weight.width_; wx++)
                    {
                        float_t dst{0};

                        size_t idx = 0;
                        idx = params.in_padded.get_index(wx, wy, inc);
                        const float_t *prevo = &prev_out[sample][idx];

                        idx = params.out.get_index(0, 0, outc);
                        const float_t *delta = &curr_delta[sample][idx];

                        if (params.w_stride > 1)
                        {
                            for (size_t y = 0; y < params.out.height_;
                                 y++)
                            {
                                size_t prevo_idx = y *
                                        params.in_padded.width_ *
                                        params.h_stride;
                                size_t delta_idx = y * params.out.width_;

                                for (size_t x = 0; x < params.out.width_;
                                     x++)
                                {
                                    dst += prevo[prevo_idx+x*
                                            params.w_stride] *
                                            delta[delta_idx + x];
                                }
                            }
                        }
                        else
                        {
                            for(size_t y = 0; y < params.out.height_;
                                y++)
                            {
                                dst += vectorize::dot(
                                            prevo + y *
                                            params.in_padded.width_ *
                                            params.h_stride,
                                            delta+y*params.out.width_,
                                            params.out.width_);
                            }
                        }

                        idx = params.in.depth_ * outc + inc;
                        dW[sample][params.weight.get_index(wx, wy, idx)]
                                += dst;
                    }
                }
            }
        }

        // accumulate db
        if (params.has_bias) {
            for (size_t outc = 0; outc < params.out.depth_; outc++)
            {
                size_t idx            = params.out.get_index(0, 0, outc);
                const float_t *delta  = &curr_delta[sample][idx];
                const float_t *deltaa = delta + params.out.width_ * params.out.height_;
                db[sample][outc] += std::accumulate(delta, deltaa, float_t{0});
            }
        }
    });
}

void TdConvolution::forward_propagation(
        const std::vector<tiny_dnn::tensor_t *> &in,
        std::vector<tiny_dnn::tensor_t *> &out)
{
    // apply padding to the input tensor
    padding_op_.copy_and_pad_input(*in[0], cws_.prev_out_padded_);

    fwd_in_data_.resize(in.size());
    std::copy(in.begin(), in.end(), fwd_in_data_.begin());
    fwd_in_data_[0] = in_data_padded(in);

    // launch convolutional kernel
    auto params = params_.conv();

    // incomimg/outcoming data
    tiny_dnn::tensor_t &in_data  = *in[0];
    tiny_dnn::tensor_t &W        = *in[1];
    tiny_dnn::tensor_t &bias     = *in[2];
    tiny_dnn::tensor_t &out_data = *out[0];

    // initialize outputs
    fill_tensor(out_data, float_t{0});

#ifdef CNN_USE_AVX
    avx_op_forward(in_data, W[0], bias[0], out_data, params);
#else
    op_forward(in_data, W[0], bias[0], out_data, params);
#endif
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

    // launch convolutional kernel
    auto params = params_.conv();

    // incoming/outcoming data
    tiny_dnn::tensor_t &prev_out = *in_data[0];
    tiny_dnn::tensor_t &W        = *in_data[1];
    tiny_dnn::tensor_t &dW             = *in_grad[1];
    tiny_dnn::tensor_t &db             = *in_grad[2];
    tiny_dnn::tensor_t &prev_delta     = *in_grad[0];
    tiny_dnn::tensor_t &curr_delta     = *out_grad[0];

    // initalize outputs
    fill_tensor(prev_delta, float_t{0});

#ifdef CNN_USE_AVX
    avx_op_backward(prev_out, W[0], dW, db, curr_delta,
            prev_delta, params);
#else
    op_backward(prev_out, W[0], dW,
            db, curr_delta, prev_delta, params);
#endif

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
