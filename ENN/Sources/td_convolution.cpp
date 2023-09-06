#include "td_convolution.h"

TdConvolution::TdConvolution(size_t in_width, size_t in_height,
                             size_t window_width, size_t window_height,
                             size_t in_channels, size_t out_channels,
                             tiny_dnn::padding pad_type, bool has_bias,
                             size_t w_stride, size_t h_stride,
                             size_t w_dilation, size_t h_dilation)
    : TdLayer(tiny_dnn::std_input_order(has_bias))
{
    conv_set_params(tiny_dnn::shape3d(in_width, in_height, in_channels),
                    window_width, window_height, out_channels,
                    pad_type, has_bias, w_stride,
                    h_stride, w_dilation, h_dilation,
                    tiny_dnn::core::connection_table());
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
       const tiny_dnn::vec_t &W, const tiny_dnn::vec_t &bias,
       tiny_dnn::tensor_t &out_data,
       const tiny_dnn::core::conv_params &params, int s_index,
       int e_index)
{
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
        for( int sample=s_index; sample<e_index; sample++ )
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
    }//, 0u);
}

template <typename tensor_t, typename vec_t>
void TdConvolution::op_backward(const tensor_t &prev_out,
    const vec_t &W, tensor_t &dW, tensor_t &db,
    tensor_t &curr_delta, tensor_t &prev_delta,
    const tiny_dnn::core::conv_params &params,
    int s_index, int e_index)
{
    typedef typename vec_t::value_type float_t;

    for( int sample=s_index ; sample<e_index ; sample++ )
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
    }
}

void TdConvolution::forward(int s_index, int e_index)
{
    // apply padding to the input tensor
    padding_op_.copy_and_pad_input(in_edges[0]->data_,
            cws_.prev_out_padded_);

    // launch convolutional kernel
    auto params = params_.conv();

    // incomimg/outcoming data
    tiny_dnn::tensor_t &in_data  = in_edges[0]->data_;
    tiny_dnn::tensor_t &W        = in_edges[1]->data_;
    tiny_dnn::tensor_t &bias     = in_edges[2]->data_;
    tiny_dnn::tensor_t &out_data = out_edges->data_;

    // initialize outputs
    fill_tensor(out_data, float_t{0});

#ifdef CNN_USE_AVX
    avx_op_forward(in_data, W[0], bias[0],
            out_data, params, s_index, e_index);
//    tiny_dnn::kernels::conv2d_op_avx(in_data, W[0], bias[0],
//            out_data, params, true);
#else
    op_forward(in_data, W[0], bias[0], out_data, params, s_index,
            e_index);
#endif
}

void TdConvolution::backward(int s_index, int e_index)
{
    // launch convolutional kernel
    auto params = params_.conv();

    // incoming/outcoming data
    tiny_dnn::tensor_t &prev_out    = in_edges[0]->data_;
    tiny_dnn::tensor_t &W           = in_edges[1]->data_;
    tiny_dnn::tensor_t &dW          = in_edges[1]->grad_;
    tiny_dnn::tensor_t &db          = in_edges[2]->grad_;
    tiny_dnn::tensor_t &prev_delta  = in_edges[0]->grad_;
    tiny_dnn::tensor_t &curr_delta  = out_edges->grad_;

    // initalize outputs
    fill_tensor(prev_delta, float_t{0});

#ifdef CNN_USE_AVX
    avx_op_backward(prev_out, W[0], dW, db,
            curr_delta, prev_delta, params, s_index, e_index);
#else
    op_backward(prev_out, W[0], dW,
            db, curr_delta, prev_delta, params, s_index, e_index);
#endif

    // unpad deltas
    padding_op_.copy_and_unpad_delta(cws_.prev_delta_padded_,
                                     in_edges[0]->grad_);
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

tiny_dnn::tensor_t *TdConvolution::in_data_padded()
{
    if( params_.pad_type==tiny_dnn::padding::valid )
    {
        return &in_edges[0]->data_;
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

#ifdef CNN_USE_AVX
template <typename Allocator>
void TdConvolution::avx_5x5_forward_kernel(
        const tiny_dnn::core::conv_params &params,
        const std::vector<float, Allocator> &in,
        const std::vector<float, Allocator> &W,
        const std::vector<float, Allocator> &bias,
        std::vector<float, Allocator> &a)
{
    auto &out       = params.out;
    auto &in_padded = params.in_padded;
    auto &tbl       = params.tbl;
    auto w_stride   = params.w_stride;

    const size_t out_area = out.area();
    size_t oidx           = 0;
    float bias_scale      = params.has_bias ? 1.0f : 0.0f;
    const size_t stride   = params.h_stride * in_padded.width_;
    const size_t inarea   = in_padded.area();

    static const __m256i imask = _mm256_setr_epi32(
                -1, -1, -1, -1, -1, 0, 0, 0);
    // static const __m256 mask = _mm256_castsi256_ps(
    //    _mm256_setr_epi32(-1, -1,
    // -1, -1, -1, 0, 0, 0));

    const __m128 y_bias_scale = _mm_set_ss(bias_scale);
    if( out.height_==1 && out.width_==1 )
    {
        const float *pw = (const float *)&W[0];
        for( size_t o=0; o<out.depth_ ; ++o )
        {
            __m256 sum0     = _mm256_setzero_ps();
            __m256 sum1     = _mm256_setzero_ps();
            __m256 sum2     = _mm256_setzero_ps();
            __m128 sum3     = _mm_setzero_ps();
            const float *pi = (const float *)&in[0];
            for( size_t inc = 0 ; inc < params.in.depth_ ;
                 ++inc, pw += 25, pi += inarea )
            {
                if (!tbl.is_connected(o, inc))
                {
                    continue;
                }
                __m256 w0   = _mm256_loadu_ps(pw + 0);
                __m256 w1   = _mm256_loadu_ps(pw + 8);
                __m256 w2   = _mm256_loadu_ps(pw + 16);
                __m256 i0   = _mm256_loadu_ps(pi + 0);
                __m256 i1   = _mm256_loadu_ps(pi + 8);
                __m256 i2   = _mm256_loadu_ps(pi + 16);
                __m128 w3   = _mm_load_ss(pw + 24);
                __m128 i3   = _mm_load_ss(pi + 24);
                __m256 tmp0 = _mm256_mul_ps(w0, i0);
                __m256 tmp1 = _mm256_mul_ps(w1, i1);
                __m256 tmp2 = _mm256_mul_ps(w2, i2);
                __m128 tmp3 = _mm_mul_ps(w3, i3);
                sum0        = _mm256_add_ps(tmp0, sum0);
                sum1        = _mm256_add_ps(tmp1, sum1);
                sum2        = _mm256_add_ps(tmp2, sum2);
                sum3        = _mm_add_ps(tmp3, sum3);
            }
            __m256 sum  = _mm256_add_ps(_mm256_add_ps(sum0, sum1), sum2);
            __m128 b    = _mm_load_ss(&bias[o]);
            __m128 hsum = hsum256_ps(sum);
            b           = madd128_ss(b, y_bias_scale, sum3);
            _mm_store_ss(&a[o], _mm_add_ss(hsum, b));
        }
    }
    else
    {
        const size_t nblocks = out.width_ / 4;
        for (size_t o = 0; o < out.depth_; ++o, oidx += out_area)
        {
            float *pa = &a[oidx];
            // init to bias value
            float b = bias[o] * bias_scale;
            {
                size_t headSize = 0;
                __m256 b2       = _mm256_set1_ps(b);
                if (oidx & 7) {
                    headSize = 8 - (oidx & 7);
                    assert(headSize < out_area);
                    for (size_t i = 0; i < headSize; ++i) {
                        _mm_store_ss(&pa[i], _mm256_castps256_ps128(b2));
                    }
                }
                size_t cnt = (out_area - headSize) / 16;
                float *pa2 = pa + headSize;
                for (size_t i = 0; i < cnt; ++i) {
                    _mm256_store_ps(&pa2[i * 16 + 0], b2);
                    _mm256_store_ps(&pa2[i * 16 + 8], b2);
                }
                for (size_t i = headSize + cnt * 16; i < out_area; ++i) {
                    pa[i] = b;
                }
            }
            for(size_t inc = 0; inc < params.in.depth_; ++inc)
            {
                if(!tbl.is_connected(o, inc))
                {
                    continue;
                }

                const float *pw = (const float *)&W[25 * (params.in.depth_ * o + inc)];
                const float *pi = (const float *)&in[in_padded.get_index(0, 0, inc)];

                __m256 w0a = _mm256_maskload_ps(pw + 0, imask);
                __m256 w1a = _mm256_maskload_ps(pw + 5, imask);
                __m256 w2a = _mm256_maskload_ps(pw + 10, imask);
                __m256 w3a = _mm256_maskload_ps(pw + 15, imask);
                __m256 w4a = _mm256_maskload_ps(pw + 20, imask);
                __m256 w0b = leftShift<4>(w0a);
                __m256 w1b = leftShift<4>(w1a);
                __m256 w2b = leftShift<4>(w2a);
                __m256 w3b = leftShift<4>(w3a);
                __m256 w4b = leftShift<4>(w4a);
                __m256 w0c = leftShift<8>(w0a);
                __m256 w1c = leftShift<8>(w1a);
                __m256 w2c = leftShift<8>(w2a);
                __m256 w3c = leftShift<8>(w3a);
                __m256 w4c = leftShift<8>(w4a);
                __m256 w0d = leftShift<12>(w0a);
                __m256 w1d = leftShift<12>(w1a);
                __m256 w2d = leftShift<12>(w2a);
                __m256 w3d = leftShift<12>(w3a);
                __m256 w4d = leftShift<12>(w4a);
                float *ppa = pa;
                if(w_stride == 1)
                {
                    if(nblocks)
                    {
                        for(size_t y = 0; y < out.height_;
                            ++y, ppa += out.width_)
                        {
                            const float *pi0 = (pi + y * stride);
                            const float *pi1 = pi0 + 1*in_padded.width_;
                            const float *pi2 = pi0 + 2*in_padded.width_;
                            const float *pi3 = pi0 + 3*in_padded.width_;
                            const float *pi4 = pi0 + 4*in_padded.width_;
                            __m256 dst0, dst1, dst2, dst3;
                            __m256 i0       = _mm256_loadu_ps(pi0);
                            __m256 i1       = _mm256_loadu_ps(pi1);
                            __m256 i2       = _mm256_loadu_ps(pi2);
                            __m256 i3       = _mm256_loadu_ps(pi3);
                            __m256 i4       = _mm256_loadu_ps(pi4);
                            dst0            = _mm256_mul_ps(w0a, i0);
                            dst1            = _mm256_mul_ps(w0b, i0);
                            dst2            = _mm256_mul_ps(w0c, i0);
                            dst3            = _mm256_mul_ps(w0d, i0);
                            dst0            = madd256_ps(w1a, i1, dst0);
                            dst1            = madd256_ps(w1b, i1, dst1);
                            dst2            = madd256_ps(w1c, i1, dst2);
                            dst3            = madd256_ps(w1d, i1, dst3);
                            dst0            = madd256_ps(w2a, i2, dst0);
                            dst1            = madd256_ps(w2b, i2, dst1);
                            dst2            = madd256_ps(w2c, i2, dst2);
                            dst3            = madd256_ps(w2d, i2, dst3);
                            dst0            = madd256_ps(w3a, i3, dst0);
                            dst1            = madd256_ps(w3b, i3, dst1);
                            dst2            = madd256_ps(w3c, i3, dst2);
                            dst3            = madd256_ps(w3d, i3, dst3);
                            dst0            = madd256_ps(w4a, i4, dst0);
                            dst1            = madd256_ps(w4b, i4, dst1);
                            dst2            = madd256_ps(w4c, i4, dst2);
                            dst3            = madd256_ps(w4d, i4, dst3);
                            __m128 sum      = _mm_loadu_ps(ppa);
                            __m128 hsum0123 = hsum4x256_ps(dst0, dst1,
                                                           dst2, dst3);
                            sum             = _mm_add_ps(sum, hsum0123);
                            _mm_storeu_ps(ppa, sum);
                            for (size_t i = 1; i < nblocks; ++i) {
                                i0       = _mm256_loadu_ps(pi0 + i * 4);
                                i1       = _mm256_loadu_ps(pi1 + i * 4);
                                i2       = _mm256_loadu_ps(pi2 + i * 4);
                                i3       = _mm256_loadu_ps(pi3 + i * 4);
                                i4       = _mm256_loadu_ps(pi4 + i * 4);
                                dst0     = _mm256_mul_ps(w0a, i0);
                                dst1     = _mm256_mul_ps(w0b, i0);
                                dst2     = _mm256_mul_ps(w0c, i0);
                                dst3     = _mm256_mul_ps(w0d, i0);
                                dst0     = madd256_ps(w1a, i1, dst0);
                                dst1     = madd256_ps(w1b, i1, dst1);
                                dst2     = madd256_ps(w1c, i1, dst2);
                                dst3     = madd256_ps(w1d, i1, dst3);
                                dst0     = madd256_ps(w2a, i2, dst0);
                                dst1     = madd256_ps(w2b, i2, dst1);
                                dst2     = madd256_ps(w2c, i2, dst2);
                                dst3     = madd256_ps(w2d, i2, dst3);
                                dst0     = madd256_ps(w3a, i3, dst0);
                                dst1     = madd256_ps(w3b, i3, dst1);
                                dst2     = madd256_ps(w3c, i3, dst2);
                                dst3     = madd256_ps(w3d, i3, dst3);
                                dst0     = madd256_ps(w4a, i4, dst0);
                                dst1     = madd256_ps(w4b, i4, dst1);
                                dst2     = madd256_ps(w4c, i4, dst2);
                                dst3     = madd256_ps(w4d, i4, dst3);
                                sum      = _mm_loadu_ps(ppa + i * 4);
                                hsum0123 = hsum4x256_ps(dst0, dst1,
                                                        dst2, dst3);
                                sum      = _mm_add_ps(sum, hsum0123);
                                _mm_storeu_ps(ppa + i * 4, sum);
                            }
                            for (size_t x = nblocks * 4;
                                 x < out.width_; ++x)
                            {
                                sum         = _mm_load_ss(&ppa[x]);
                                i0          = _mm256_loadu_ps(pi0 + x);
                                i1          = _mm256_loadu_ps(pi1 + x);
                                i2          = _mm256_loadu_ps(pi2 + x);
                                i3          = _mm256_loadu_ps(pi3 + x);
                                i4 = _mm256_maskload_ps(pi4 + x, imask);
                                __m256 sum0 = _mm256_mul_ps(w0a, i0);
                                __m256 sum1 = _mm256_mul_ps(w1a, i1);
                                sum0        = madd256_ps(w2a, i2, sum0);
                                sum1        = madd256_ps(w3a, i3, sum1);
                                sum0        = madd256_ps(w4a, i4, sum0);
                                sum0        = _mm256_add_ps(sum0, sum1);
                                _mm_store_ss(&ppa[x], _mm_add_ss(sum,
                                                                 hsum256_ps(sum0)));
                            }     // x loop
                        }       // y loop
                    }
                    else
                    {  // if (nblocks) {
                        for (size_t y = 0; y < out.height_;
                             ++y, ppa += out.width_)
                        {
                            const float *pi0 = (pi + y * stride);
                            const float *pi1 = pi0 + 1*in_padded.width_;
                            const float *pi2 = pi0 + 2*in_padded.width_;
                            const float *pi3 = pi0 + 3*in_padded.width_;
                            const float *pi4 = pi0 + 4*in_padded.width_;
                            for (size_t x = 0; x < out.width_; ++x)
                            {
                                __m128 sum  = _mm_load_ss(&ppa[x]);
                                __m256 i0   = _mm256_loadu_ps(pi0 + x);
                                __m256 i1   = _mm256_loadu_ps(pi1 + x);
                                __m256 i2   = _mm256_loadu_ps(pi2 + x);
                                __m256 i3   = _mm256_loadu_ps(pi3 + x);
                                __m256 i4   = _mm256_maskload_ps(pi4 + x, imask);
                                __m256 sum0 = _mm256_mul_ps(w0a, i0);
                                __m256 sum1 = _mm256_mul_ps(w1a, i1);
                                sum0        = madd256_ps(w2a, i2, sum0);
                                sum1        = madd256_ps(w3a, i3, sum1);
                                sum0        = madd256_ps(w4a, i4, sum0);
                                sum0        = _mm256_add_ps(sum0, sum1);
                                _mm_store_ss(&ppa[x], _mm_add_ss(sum,
                                                                 hsum256_ps(sum0)));
                            }  // x loop
                        }    // y loop
                    }
                }
                else
                {  // if (w_stride == 1) {
                    for (size_t y = 0; y < out.height_;
                         ++y, ppa += out.width_)
                    {
                        const float *pi0 = (pi + y * stride);
                        const float *pi1 = pi0 + 1 * in_padded.width_;
                        const float *pi2 = pi0 + 2 * in_padded.width_;
                        const float *pi3 = pi0 + 3 * in_padded.width_;
                        const float *pi4 = pi0 + 4 * in_padded.width_;
                        for (size_t x = 0; x < out.width_; ++x)
                        {
                            __m128 sum  = _mm_load_ss(&ppa[x]);
                            __m256 i0   = _mm256_loadu_ps(pi0);
                            __m256 i1   = _mm256_loadu_ps(pi1);
                            __m256 i2   = _mm256_loadu_ps(pi2);
                            __m256 i3   = _mm256_loadu_ps(pi3);
                            __m256 i4   = _mm256_maskload_ps(pi4, imask);
                            __m256 sum0 = _mm256_mul_ps(w0a, i0);
                            __m256 sum1 = _mm256_mul_ps(w1a, i1);
                            sum0        = madd256_ps(w2a, i2, sum0);
                            sum1        = madd256_ps(w3a, i3, sum1);
                            sum0        = madd256_ps(w4a, i4, sum0);
                            sum0        = _mm256_add_ps(sum0, sum1);
                            _mm_store_ss(&ppa[x], _mm_add_ss(sum,
                                                             hsum256_ps(sum0)));
                            pi0 += w_stride;
                            pi1 += w_stride;
                            pi2 += w_stride;
                            pi3 += w_stride;
                            pi4 += w_stride;
                        }  // x loop
                    }    // y loop
                }
            }  // in depth loop
        }    // out depth loop
    }      // else
}  // avx_conv2d_5x5_kernel float ver

void TdConvolution::avx_op_forward(const tiny_dnn::tensor_t &in_data,
                                   const tiny_dnn::vec_t &W,
                                   const tiny_dnn::vec_t &bias,
                                   tiny_dnn::tensor_t &out_data,
                                   const tiny_dnn::core::conv_params &params,
                                   int s_index, int e_index)
{
//    int parallelized = true;
    if( params.weight.height_==5 && params.weight.width_==5 )
    {
        // @todo consider better parallelization
        for( int i=s_index ; i<e_index ; i++ )
        {
            avx_5x5_forward_kernel(params, in_data[i], W, bias,
                                   out_data[i]);
        }
        return;
    }
    op_forward(in_data, W, bias, out_data, params, s_index, e_index);
}

// float ver
template <typename Allocator>
void TdConvolution::avx_accumulate_db(const tiny_dnn::index3d<size_t> &out,
                                  const std::vector<float, Allocator> &curr_delta,
                                  std::vector<float, Allocator> &db)
{
    if (out.width_ == 1 && out.height_ == 1)
    {
        size_t nblocks = out.depth_ / 8;
        for (size_t i = 0; i < nblocks; ++i)
        {
            _mm256_storeu_ps(&db[i * 8],
                    _mm256_add_ps(_mm256_loadu_ps(&db[i * 8]),
                    _mm256_loadu_ps(&curr_delta[i * 8])));
        }
        for (size_t outc = nblocks * 8; outc < out.depth_; ++outc)
        {
            db[outc] += curr_delta[outc];
        }
    }
    else
    {
        auto area        = out.area();
        size_t n8        = area / 64;
        size_t n4        = (area % 64) / 32;
        size_t n2        = (area % 32) / 16;
        size_t n1        = (area % 16) / 8;
        size_t remainder = area & 7;
        // prepare load-mask beforehand
        static const int32_t masks[] = {
            -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0,
        };
        __m256i mask = _mm256_loadu_si256((const __m256i *)(masks + 8 - remainder));
        for (size_t outc = 0; outc < out.depth_; ++outc)
        {
            size_t idx         = out.get_index(0, 0, outc);
            const float *delta = &curr_delta[idx];
            __m256 sum0        = _mm256_setzero_ps();
            __m256 sum1        = _mm256_setzero_ps();
            __m256 sum2        = _mm256_setzero_ps();
            __m256 sum3        = _mm256_setzero_ps();
            __m256 sum4        = _mm256_setzero_ps();
            __m256 sum5        = _mm256_setzero_ps();
            __m256 sum6        = _mm256_setzero_ps();
            __m256 sum7        = _mm256_setzero_ps();
            for (size_t i = 0; i < n8; ++i)
            {
                sum0 = _mm256_add_ps(sum0, _mm256_loadu_ps(delta + i * 64 + 0));
                sum1 = _mm256_add_ps(sum1, _mm256_loadu_ps(delta + i * 64 + 8));
                sum2 = _mm256_add_ps(sum2, _mm256_loadu_ps(delta + i * 64 + 16));
                sum3 = _mm256_add_ps(sum3, _mm256_loadu_ps(delta + i * 64 + 24));
                sum4 = _mm256_add_ps(sum4, _mm256_loadu_ps(delta + i * 64 + 32));
                sum5 = _mm256_add_ps(sum5, _mm256_loadu_ps(delta + i * 64 + 40));
                sum6 = _mm256_add_ps(sum6, _mm256_loadu_ps(delta + i * 64 + 48));
                sum7 = _mm256_add_ps(sum7, _mm256_loadu_ps(delta + i * 64 + 56));
            }
            delta += n8 * 64;
            if (n4)
            {
                sum0 = _mm256_add_ps(sum0, _mm256_loadu_ps(delta + 0));
                sum1 = _mm256_add_ps(sum1, _mm256_loadu_ps(delta + 8));
                sum2 = _mm256_add_ps(sum2, _mm256_loadu_ps(delta + 16));
                sum3 = _mm256_add_ps(sum3, _mm256_loadu_ps(delta + 24));
                delta += 32;
            }
            if (n2)
            {
                sum4 = _mm256_add_ps(sum4, _mm256_loadu_ps(delta + 0));
                sum5 = _mm256_add_ps(sum5, _mm256_loadu_ps(delta + 8));
                delta += 16;
            }
            if (n1)
            {
                sum6 = _mm256_add_ps(sum6, _mm256_loadu_ps(delta));
                delta += 8;
            }
            sum0 = _mm256_add_ps(sum0, sum1);
            sum2 = _mm256_add_ps(sum2, sum3);
            sum4 = _mm256_add_ps(sum4, sum5);
            sum6 = _mm256_add_ps(sum6, sum7);
            sum1 = _mm256_maskload_ps(delta, mask);
            sum0 = _mm256_add_ps(sum0, sum2);
            sum4 = _mm256_add_ps(sum4, sum6);
            sum0 = _mm256_add_ps(sum0, sum4);
            sum0 = _mm256_add_ps(sum0, sum1);
            db[outc] += _mm_cvtss_f32(hsum256_ps(sum0));
        }
    }
}  // accumulate_db

// float ver
template <typename Allocator>
void TdConvolution::avx_accumulate_dw(const tiny_dnn::core::conv_params &params,
                                  const std::vector<float, Allocator> &prev_out,
                                  const std::vector<float, Allocator> &curr_delta,
                                  std::vector<float, Allocator> &dW,
                                  std::vector<float, Allocator> &db)
{
    CNN_UNREFERENCED_PARAMETER(db);
    auto &in                    = params.in;
    auto &out                   = params.out;
    auto &in_padded             = params.in_padded;
    auto &tbl                   = params.tbl;
    auto w_stride               = params.w_stride;
    const size_t in_padded_area = in_padded.area();
    static const __m256i imask  = _mm256_setr_epi32(-1, -1, -1, -1, -1, 0, 0, 0);

    if (out.width_ == 1 && out.height_ == 1)
    {
        const float *pprev_out = &prev_out[0];
        alignas(32) float floats[28];
        for (size_t inc = 0; inc < in.depth_; ++inc, pprev_out += in_padded_area)
        {
            size_t in_padded_width = in_padded.width_;
            _mm256_store_ps(&floats[0],
                    _mm256_loadu_ps(pprev_out + in_padded_width * 0));
            _mm256_storeu_ps(&floats[5],
                    _mm256_loadu_ps(pprev_out + in_padded_width * 1));
            _mm256_storeu_ps(&floats[10],
                    _mm256_loadu_ps(pprev_out + in_padded_width * 2));
            _mm256_storeu_ps(&floats[15],
                    _mm256_loadu_ps(pprev_out + in_padded_width * 3));
            _mm256_storeu_ps(&floats[20], _mm256_maskload_ps(
                        pprev_out + in_padded_width * 4, imask));
            __m256 prevos0    = _mm256_load_ps(&floats[0]);
            __m256 prevos1    = _mm256_load_ps(&floats[8]);
            __m256 prevos2    = _mm256_load_ps(&floats[16]);
            __m128 prevos3    = _mm_load_ss(&floats[24]);
            size_t widx       = 25 * inc;
            size_t widx_delta = 25 * in.depth_;
            float *pdW        = &dW[widx];
            for (size_t outc = 0; outc < out.depth_; outc++, pdW += widx_delta)
            {
                if (!tbl.is_connected(outc, inc))
                {
                    continue;
                }
                __m256 delta = _mm256_broadcast_ss(&curr_delta[outc]);
                __m256 w0    = _mm256_loadu_ps(pdW + 0);
                __m256 w1    = _mm256_loadu_ps(pdW + 8);
                __m256 w2    = _mm256_loadu_ps(pdW + 16);
                __m128 w3    = _mm_load_ss(pdW + 24);
                w0           = madd256_ps(prevos0, delta, w0);
                w1           = madd256_ps(prevos1, delta, w1);
                w2           = madd256_ps(prevos2, delta, w2);
                w3           = madd128_ss(prevos3, _mm256_castps256_ps128(delta), w3);
                _mm256_storeu_ps(pdW + 0, w0);
                _mm256_storeu_ps(pdW + 8, w1);
                _mm256_storeu_ps(pdW + 16, w2);
                _mm_store_ss(pdW + 24, w3);
            }
        }
    }
    else
    {
        // prepare load-mask beforehand
        const size_t nblocks         = out.width_ >> 3;
        static const int32_t masks[] = {
            -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0,
        };
        const size_t remainder = out.width_ & 7;
        __m256i mask = _mm256_loadu_si256((const __m256i *)(masks + 8 - remainder));
        auto &weight = params.weight;
        size_t prevo_delta      = in_padded.width_ * params.h_stride;
        const size_t out_width  = out.width_;
        const size_t out_height = out.height_;
        assert(1 < out_width);
        assert(1 < out_height);
        __m256 sum0, sum1, sum2, sum3, sum4;
        if (w_stride > 1)
        {
            for (size_t inc = 0; inc < in.depth_; ++inc)
            {
                for (size_t outc = 0; outc < out.depth_; ++outc)
                {
                    const float *delta = &curr_delta[out.get_index(0, 0, outc)];
                    if (!tbl.is_connected(outc, inc))
                    {
                        continue;
                    }
                    size_t widx = weight.get_index(0, 0, in.depth_ * outc + inc);
                    // weight.height_
                    for (size_t wy = 0; wy < 5; ++wy)
                    {
                        // weight.width_
                        for (size_t wx = 0; wx < 5; ++wx, ++widx)
                        {
                            size_t prev_out_idx = in_padded.get_index(wx, wy, inc);
                            const float *prevo  = &prev_out[prev_out_idx];
                            float_t dst{0};
                            for (size_t y = 0, prevo_idx = 0, delta_idx = 0; y < out_height;
                                 ++y, prevo_idx += prevo_delta, delta_idx += out_width)
                            {
                                for (size_t x = 0; x < out_width; ++x)
                                {
                                    dst += prevo[prevo_idx + x * params.w_stride] *
                                            delta[delta_idx + x];
                                }
                            }
                            dW[widx] += dst;
                        }  // for wx
                    }    // for wy
                }      // for outc
            }        // for inc
        }
        else if (nblocks == 1 && remainder != 0)
        {
            for (size_t inc = 0; inc < in.depth_; ++inc)
            {
                for (size_t outc = 0; outc < out.depth_; ++outc)
                {
                    if (!tbl.is_connected(outc, inc))
                    {
                        continue;
                    }
                    const float *delta = &curr_delta[out.get_index(0, 0, outc)];
                    size_t widx        = weight.get_index(0, 0, in.depth_ * outc + inc);
                    float *pdw         = &dW[widx];
                    // weight.height_
                    for (size_t wy = 0; wy < 5; ++wy)
                    {
                        size_t prev_out_idx = in_padded.get_index(0, wy, inc);
                        const float *pa     = &prev_out[prev_out_idx];
                        const float *pb     = delta;
                        // y = 0
                        sum0 = sum1 = sum2 = sum3 = sum4 = _mm256_setzero_ps();
                        for (size_t y = 0; y < out_height; ++y)
                        {
                            // vectorize::dot
                            __m256 a0 = _mm256_loadu_ps(pa + 0);
                            __m256 a1 = _mm256_loadu_ps(pa + 1);
                            __m256 a2 = _mm256_loadu_ps(pa + 2);
                            __m256 a3 = _mm256_loadu_ps(pa + 3);
                            __m256 a4 = _mm256_loadu_ps(pa + 4);
                            __m256 b  = _mm256_loadu_ps(pb);
                            sum0      = madd256_ps(a0, b, sum0);
                            sum1      = madd256_ps(a1, b, sum1);
                            sum2      = madd256_ps(a2, b, sum2);
                            sum3      = madd256_ps(a3, b, sum3);
                            sum4      = madd256_ps(a4, b, sum4);
                            a0        = _mm256_maskload_ps(pa + 010, mask);
                            a1        = _mm256_maskload_ps(pa + 011, mask);
                            a2        = _mm256_maskload_ps(pa + 012, mask);
                            a3        = _mm256_maskload_ps(pa + 013, mask);
                            a4        = _mm256_maskload_ps(pa + 014, mask);
                            b         = _mm256_maskload_ps(pb + 010, mask);
                            sum0      = madd256_ps(a0, b, sum0);
                            sum1      = madd256_ps(a1, b, sum1);
                            sum2      = madd256_ps(a2, b, sum2);
                            sum3      = madd256_ps(a3, b, sum3);
                            sum4      = madd256_ps(a4, b, sum4);
                            pa += prevo_delta;
                            pb += out_width;
                        }
                        _mm_storeu_ps(pdw + wy * 5,
                                      _mm_add_ps(_mm_loadu_ps(pdw + wy * 5),
                                                 hsum4x256_ps(sum0, sum1, sum2, sum3)));
                        _mm_store_ss(
                                    pdw + wy * 5 + 4,
                                    _mm_add_ss(_mm_load_ss(pdw + wy * 5 + 4), hsum256_ps(sum4)));
                    }  // for wy
                }    // for outc
            }      // for inc
        }
        else if (nblocks > 1 && remainder != 0)
        {
            for (size_t inc = 0; inc < in.depth_; ++inc)
            {
                for (size_t outc = 0; outc < out.depth_; ++outc)
                {
                    if (!tbl.is_connected(outc, inc))
                    {
                        continue;
                    }
                    const float *delta = &curr_delta[out.get_index(0, 0, outc)];
                    size_t widx        = weight.get_index(0, 0, in.depth_ * outc + inc);
                    float *pdw         = &dW[widx];
                    // weight.height_
                    for (size_t wy = 0; wy < 5; ++wy)
                    {
                        size_t prev_out_idx = in_padded.get_index(0, wy, inc);
                        const float *pa     = &prev_out[prev_out_idx];
                        const float *pb     = delta;
                        sum0 = sum1 = sum2 = sum3 = sum4 = _mm256_setzero_ps();
                        for (size_t y = 0; y < out_height;
                             ++y, pa += prevo_delta, pb += out_width)
                        {
                            // vectorize::dot
                            __m256 a0 = _mm256_loadu_ps(pa + 0);
                            __m256 a1 = _mm256_loadu_ps(pa + 1);
                            __m256 a2 = _mm256_loadu_ps(pa + 2);
                            __m256 a3 = _mm256_loadu_ps(pa + 3);
                            __m256 a4 = _mm256_loadu_ps(pa + 4);
                            __m256 b  = _mm256_loadu_ps(pb);
                            sum0      = madd256_ps(a0, b, sum0);
                            sum1      = madd256_ps(a1, b, sum1);
                            sum2      = madd256_ps(a2, b, sum2);
                            sum3      = madd256_ps(a3, b, sum3);
                            sum4      = madd256_ps(a4, b, sum4);
                            a0        = _mm256_loadu_ps(pa + 010);
                            a1        = _mm256_loadu_ps(pa + 011);
                            a2        = _mm256_loadu_ps(pa + 012);
                            a3        = _mm256_loadu_ps(pa + 013);
                            a4        = _mm256_loadu_ps(pa + 014);
                            b         = _mm256_loadu_ps(pb + 010);
                            sum0      = madd256_ps(a0, b, sum0);
                            sum1      = madd256_ps(a1, b, sum1);
                            sum2      = madd256_ps(a2, b, sum2);
                            sum3      = madd256_ps(a3, b, sum3);
                            sum4      = madd256_ps(a4, b, sum4);
                            for (size_t i = 2; i < nblocks; ++i) {
                                a0   = _mm256_loadu_ps(pa + 8 * i + 0);
                                a1   = _mm256_loadu_ps(pa + 8 * i + 1);
                                a2   = _mm256_loadu_ps(pa + 8 * i + 2);
                                a3   = _mm256_loadu_ps(pa + 8 * i + 3);
                                a4   = _mm256_loadu_ps(pa + 8 * i + 4);
                                b    = _mm256_loadu_ps(pb + 8 * i);
                                sum0 = madd256_ps(a0, b, sum0);
                                sum1 = madd256_ps(a1, b, sum1);
                                sum2 = madd256_ps(a2, b, sum2);
                                sum3 = madd256_ps(a3, b, sum3);
                                sum4 = madd256_ps(a4, b, sum4);
                            }
                            a0   = _mm256_maskload_ps(pa + 8 * nblocks + 0, mask);
                            a1   = _mm256_maskload_ps(pa + 8 * nblocks + 1, mask);
                            a2   = _mm256_maskload_ps(pa + 8 * nblocks + 2, mask);
                            a3   = _mm256_maskload_ps(pa + 8 * nblocks + 3, mask);
                            a4   = _mm256_maskload_ps(pa + 8 * nblocks + 4, mask);
                            b    = _mm256_maskload_ps(pb + 8 * nblocks, mask);
                            sum0 = madd256_ps(a0, b, sum0);
                            sum1 = madd256_ps(a1, b, sum1);
                            sum2 = madd256_ps(a2, b, sum2);
                            sum3 = madd256_ps(a3, b, sum3);
                            sum4 = madd256_ps(a4, b, sum4);
                        }
                        _mm_storeu_ps(pdw + wy * 5,
                                      _mm_add_ps(_mm_loadu_ps(pdw + wy * 5),
                                                 hsum4x256_ps(sum0, sum1, sum2, sum3)));
                        _mm_store_ss(
                                    pdw + wy * 5 + 4,
                                    _mm_add_ss(_mm_load_ss(pdw + wy * 5 + 4), hsum256_ps(sum4)));
                    }  // for wy
                }    // for outc
            }      // for inc
        }
        else if (nblocks == 0)
        {
            assert(remainder != 0);
            for (size_t inc = 0; inc < in.depth_; ++inc)
            {
                for (size_t outc = 0; outc < out.depth_; ++outc)
                {
                    if (!tbl.is_connected(outc, inc))
                    {
                        continue;
                    }
                    const float *delta = &curr_delta[out.get_index(0, 0, outc)];
                    size_t widx        = weight.get_index(0, 0, in.depth_ * outc + inc);
                    float *pdw         = &dW[widx];
                    // weight.height_
                    for (size_t wy = 0; wy < 5; ++wy)
                    {
                        size_t prev_out_idx = in_padded.get_index(0, wy, inc);
                        const float *pa     = &prev_out[prev_out_idx];
                        const float *pb     = delta;
                        // vectorize::dot
                        sum0 = sum1 = sum2 = sum3 = sum4 = _mm256_setzero_ps();
                        for (size_t y = 0; y < out_height; ++y)
                        {
                            // vectorize::dot
                            __m256 a0 = _mm256_maskload_ps(pa + 0, mask);
                            __m256 a1 = _mm256_maskload_ps(pa + 1, mask);
                            __m256 a2 = _mm256_maskload_ps(pa + 2, mask);
                            __m256 a3 = _mm256_maskload_ps(pa + 3, mask);
                            __m256 a4 = _mm256_maskload_ps(pa + 4, mask);
                            __m256 b  = _mm256_maskload_ps(pb, mask);
                            sum0      = madd256_ps(a0, b, sum0);
                            sum1      = madd256_ps(a1, b, sum1);
                            sum2      = madd256_ps(a2, b, sum2);
                            sum3      = madd256_ps(a3, b, sum3);
                            sum4      = madd256_ps(a4, b, sum4);
                            pa += prevo_delta;
                            pb += out_width;
                        }
                        _mm_storeu_ps(pdw + wy * 5,
                                      _mm_add_ps(_mm_loadu_ps(pdw + wy * 5),
                                                 hsum4x256_ps(sum0, sum1, sum2, sum3)));
                        _mm_store_ss(
                                    pdw + wy * 5 + 4,
                                    _mm_add_ss(_mm_load_ss(pdw + wy * 5 + 4), hsum256_ps(sum4)));
                    }  // for wy
                }    // for outc
            }      // for inc
        }
        else if (nblocks == 1)
        {
            assert(remainder == 0);
            for (size_t inc = 0; inc < in.depth_; ++inc)
            {
                for (size_t outc = 0; outc < out.depth_; ++outc)
                {
                    if (!tbl.is_connected(outc, inc))
                    {
                        continue;
                    }
                    const float *delta = &curr_delta[out.get_index(0, 0, outc)];
                    size_t widx        = weight.get_index(0, 0, in.depth_ * outc + inc);
                    float *pdw         = &dW[widx];
                    // weight.height_
                    for (size_t wy = 0; wy < 5; ++wy)
                    {
                        size_t prev_out_idx = in_padded.get_index(0, wy, inc);
                        const float *pa     = &prev_out[prev_out_idx];
                        const float *pb     = delta;
                        // vectorize::dot
                        sum0 = sum1 = sum2 = sum3 = sum4 = _mm256_setzero_ps();
                        for (size_t y = 0; y < out_height; ++y)
                        {
                            // vectorize::dot
                            __m256 a0 = _mm256_loadu_ps(pa + 0);
                            __m256 a1 = _mm256_loadu_ps(pa + 1);
                            __m256 a2 = _mm256_loadu_ps(pa + 2);
                            __m256 a3 = _mm256_loadu_ps(pa + 3);
                            __m256 a4 = _mm256_loadu_ps(pa + 4);
                            __m256 b  = _mm256_loadu_ps(pb);
                            sum0      = madd256_ps(a0, b, sum0);
                            sum1      = madd256_ps(a1, b, sum1);
                            sum2      = madd256_ps(a2, b, sum2);
                            sum3      = madd256_ps(a3, b, sum3);
                            sum4      = madd256_ps(a4, b, sum4);
                            pa += prevo_delta;
                            pb += out_width;
                        }
                        _mm_storeu_ps(pdw + wy * 5,
                                      _mm_add_ps(_mm_loadu_ps(pdw + wy * 5),
                                                 hsum4x256_ps(sum0, sum1, sum2, sum3)));
                        _mm_store_ss(
                                    pdw + wy * 5 + 4,
                                    _mm_add_ss(_mm_load_ss(pdw + wy * 5 + 4), hsum256_ps(sum4)));
                    }  // for wy
                }    // for outc
            }      // for inc
        }
        else
        {
            assert(nblocks > 1);
            assert(remainder == 0);
            for (size_t inc = 0; inc < in.depth_; ++inc)
            {
                for (size_t outc = 0; outc < out.depth_; ++outc)
                {
                    if (!tbl.is_connected(outc, inc))
                    {
                        continue;
                    }
                    const float *delta = &curr_delta[out.get_index(0, 0, outc)];
                    size_t widx        = weight.get_index(0, 0, in.depth_ * outc + inc);
                    float *pdw         = &dW[widx];
                    // weight.height_
                    for (size_t wy = 0; wy < 5; ++wy)
                    {
                        size_t prev_out_idx = in_padded.get_index(0, wy, inc);
                        const float *pa     = &prev_out[prev_out_idx];
                        const float *pb     = delta;
                        // vectorize::dot
                        sum0 = sum1 = sum2 = sum3 = sum4 = _mm256_setzero_ps();
                        for (size_t y = 0; y < out_height; ++y)
                        {
                            // vectorize::dot
                            __m256 a0 = _mm256_loadu_ps(pa + 0);
                            __m256 a1 = _mm256_loadu_ps(pa + 1);
                            __m256 a2 = _mm256_loadu_ps(pa + 2);
                            __m256 a3 = _mm256_loadu_ps(pa + 3);
                            __m256 a4 = _mm256_loadu_ps(pa + 4);
                            __m256 b  = _mm256_loadu_ps(pb);
                            sum0      = madd256_ps(a0, b, sum0);
                            sum1      = madd256_ps(a1, b, sum1);
                            sum2      = madd256_ps(a2, b, sum2);
                            sum3      = madd256_ps(a3, b, sum3);
                            sum4      = madd256_ps(a4, b, sum4);
                            a0        = _mm256_loadu_ps(pa + 010);
                            a1        = _mm256_loadu_ps(pa + 011);
                            a2        = _mm256_loadu_ps(pa + 012);
                            a3        = _mm256_loadu_ps(pa + 013);
                            a4        = _mm256_loadu_ps(pa + 014);
                            b         = _mm256_loadu_ps(pb + 010);
                            sum0      = madd256_ps(a0, b, sum0);
                            sum1      = madd256_ps(a1, b, sum1);
                            sum2      = madd256_ps(a2, b, sum2);
                            sum3      = madd256_ps(a3, b, sum3);
                            sum4      = madd256_ps(a4, b, sum4);
                            for (size_t i = 2; i < nblocks; ++i)
                            {
                                a0   = _mm256_loadu_ps(pa + 8 * i + 0);
                                a1   = _mm256_loadu_ps(pa + 8 * i + 1);
                                a2   = _mm256_loadu_ps(pa + 8 * i + 2);
                                a3   = _mm256_loadu_ps(pa + 8 * i + 3);
                                a4   = _mm256_loadu_ps(pa + 8 * i + 4);
                                b    = _mm256_loadu_ps(pb + 8 * i);
                                sum0 = madd256_ps(a0, b, sum0);
                                sum1 = madd256_ps(a1, b, sum1);
                                sum2 = madd256_ps(a2, b, sum2);
                                sum3 = madd256_ps(a3, b, sum3);
                                sum4 = madd256_ps(a4, b, sum4);
                            }
                            pa += prevo_delta;
                            pb += out_width;
                        }
                        _mm_storeu_ps(pdw + wy * 5,
                                      _mm_add_ps(_mm_loadu_ps(pdw + wy * 5),
                                                 hsum4x256_ps(sum0, sum1, sum2, sum3)));
                        _mm_store_ss(
                                    pdw + wy * 5 + 4,
                                    _mm_add_ss(_mm_load_ss(pdw + wy * 5 + 4), hsum256_ps(sum4)));
                    }  // for wy
                }    // for outc
            }      // for inc
        }
    }  // else
}  // accumulate_dw

// float ver
template <typename Allocator>
void TdConvolution::avx_5x5_backward_kernel(
        const tiny_dnn::core::conv_params &params,
        const std::vector<float, Allocator> &prev_out,
        const std::vector<float, Allocator> &W,
        std::vector<float, Allocator> &dW,
        std::vector<float, Allocator> &db,
        std::vector<float, Allocator> &curr_delta,
        std::vector<float, Allocator> *prev_delta)
{
    auto &in                    = params.in;
    auto &out                   = params.out;
    auto &in_padded             = params.in_padded;
    auto &tbl                   = params.tbl;
    auto w_stride               = params.w_stride;
    const size_t in_padded_area = in_padded.area();
    float *pdelta_dst_org       = &(*prev_delta)[0];
    const size_t h_stride2      = params.h_stride * in_padded.width_;
    static const __m256i imask  = _mm256_setr_epi32(-1, -1, -1, -1, -1, 0, 0, 0);
    static const __m256 mask =
            _mm256_castsi256_ps(_mm256_setr_epi32(-1, -1, -1, -1, -1, 0, 0, 0));
    const size_t out_width  = out.width_;
    const size_t out_height = out.height_;
    // propagate delta to previous layer
    if (w_stride == 1 && out_width >= 4)
    {
        const size_t nblocks = out_width / 4;
        if (out_width % 4)
        {
            for (size_t inc = 0; inc < in.depth_;
                 ++inc, pdelta_dst_org += in_padded_area)
            {
                for (size_t outc = 0; outc < out.depth_; ++outc)
                {
                    if (!tbl.is_connected(outc, inc))
                    {
                        continue;
                    }
                    const float *pw         = &W[25 * (in.depth_ * outc + inc)];
                    const float *pdelta_src = &curr_delta[out.get_index(0, 0, outc)];
                    float *pdelta_dst       = pdelta_dst_org;
                    __m256 w0a = _mm256_and_ps(_mm256_loadu_ps(pw + 0), mask);
                    __m256 w1a = _mm256_and_ps(_mm256_loadu_ps(pw + 5), mask);
                    __m256 w2a = _mm256_and_ps(_mm256_loadu_ps(pw + 10), mask);
                    __m256 w3a = _mm256_and_ps(_mm256_loadu_ps(pw + 15), mask);
                    __m256 w4a = _mm256_maskload_ps(pw + 20, imask);
                    __m256 w0b = leftShift<4>(w0a);
                    __m256 w1b = leftShift<4>(w1a);
                    __m256 w2b = leftShift<4>(w2a);
                    __m256 w3b = leftShift<4>(w3a);
                    __m256 w4b = leftShift<4>(w4a);
                    __m256 w0c = leftShift<8>(w0a);
                    __m256 w1c = leftShift<8>(w1a);
                    __m256 w2c = leftShift<8>(w2a);
                    __m256 w3c = leftShift<8>(w3a);
                    __m256 w4c = leftShift<8>(w4a);
                    __m256 w0d = leftShift<12>(w0a);
                    __m256 w1d = leftShift<12>(w1a);
                    __m256 w2d = leftShift<12>(w2a);
                    __m256 w3d = leftShift<12>(w3a);
                    __m256 w4d = leftShift<12>(w4a);
                    for (size_t y = 0; y < out_height;
                         ++y, pdelta_src += out_width, pdelta_dst += h_stride2)
                    {
                        float *delta_dst0 = pdelta_dst;
                        float *delta_dst1 = &pdelta_dst[in_padded.width_ * 1];
                        float *delta_dst2 = &pdelta_dst[in_padded.width_ * 2];
                        float *delta_dst3 = &pdelta_dst[in_padded.width_ * 3];
                        float *delta_dst4 = &pdelta_dst[in_padded.width_ * 4];
                        for (size_t n = 0; n < nblocks; ++n)
                        {
                            __m256 delta_src =
                                    _mm256_broadcast_ps((const __m128 *)(pdelta_src + n * 4));
                            __m256 dst0 = _mm256_loadu_ps(delta_dst0 + 4 * n);
                            __m256 dst1 = _mm256_loadu_ps(delta_dst1 + 4 * n);
                            __m256 dst2 = _mm256_loadu_ps(delta_dst2 + 4 * n);
                            __m256 dst3 = _mm256_loadu_ps(delta_dst3 + 4 * n);
                            __m256 dst4 = _mm256_loadu_ps(delta_dst4 + 4 * n);
                            __m256 delta_src0 =
                                    _mm256_permute_ps(delta_src, _MM_SHUFFLE(0, 0, 0, 0));
                            __m256 delta_src1 =
                                    _mm256_permute_ps(delta_src, _MM_SHUFFLE(1, 1, 1, 1));
                            __m256 delta_src2 =
                                    _mm256_permute_ps(delta_src, _MM_SHUFFLE(2, 2, 2, 2));
                            __m256 delta_src3 =
                                    _mm256_permute_ps(delta_src, _MM_SHUFFLE(3, 3, 3, 3));
                            dst0 = madd256_ps(w0a, delta_src0, dst0);
                            dst1 = madd256_ps(w1a, delta_src0, dst1);
                            dst2 = madd256_ps(w2a, delta_src0, dst2);
                            dst3 = madd256_ps(w3a, delta_src0, dst3);
                            dst4 = madd256_ps(w4a, delta_src0, dst4);
                            dst0 = madd256_ps(w0b, delta_src1, dst0);
                            dst1 = madd256_ps(w1b, delta_src1, dst1);
                            dst2 = madd256_ps(w2b, delta_src1, dst2);
                            dst3 = madd256_ps(w3b, delta_src1, dst3);
                            dst4 = madd256_ps(w4b, delta_src1, dst4);
                            dst0 = madd256_ps(w0c, delta_src2, dst0);
                            dst1 = madd256_ps(w1c, delta_src2, dst1);
                            dst2 = madd256_ps(w2c, delta_src2, dst2);
                            dst3 = madd256_ps(w3c, delta_src2, dst3);
                            dst4 = madd256_ps(w4c, delta_src2, dst4);
                            dst0 = madd256_ps(w0d, delta_src3, dst0);
                            _mm256_storeu_ps(delta_dst0 + 4 * n, dst0);
                            dst1 = madd256_ps(w1d, delta_src3, dst1);
                            _mm256_storeu_ps(delta_dst1 + 4 * n, dst1);
                            dst2 = madd256_ps(w2d, delta_src3, dst2);
                            _mm256_storeu_ps(delta_dst2 + 4 * n, dst2);
                            dst3 = madd256_ps(w3d, delta_src3, dst3);
                            _mm256_storeu_ps(delta_dst3 + 4 * n, dst3);
                            dst4 = madd256_ps(w4d, delta_src3, dst4);
                            _mm256_storeu_ps(delta_dst4 + 4 * n, dst4);
                        }  // for nblocks
                        for (size_t x = nblocks * 4; x < out_width; ++x)
                        {
                            __m256 delta_src = _mm256_broadcast_ss(pdelta_src + x);
                            __m256 dst0      = _mm256_loadu_ps(delta_dst0 + x);
                            __m256 dst1      = _mm256_loadu_ps(delta_dst1 + x);
                            __m256 dst2      = _mm256_loadu_ps(delta_dst2 + x);
                            __m256 dst3      = _mm256_loadu_ps(delta_dst3 + x);
                            __m256 dst4      = _mm256_maskload_ps(delta_dst4 + x, imask);
                            dst0             = madd256_ps(w0a, delta_src, dst0);
                            dst1             = madd256_ps(w1a, delta_src, dst1);
                            dst2             = madd256_ps(w2a, delta_src, dst2);
                            dst3             = madd256_ps(w3a, delta_src, dst3);
                            dst4             = madd256_ps(w4a, delta_src, dst4);
                            _mm256_maskstore_ps(delta_dst0 + x, imask, dst0);
                            _mm256_maskstore_ps(delta_dst1 + x, imask, dst1);
                            _mm256_maskstore_ps(delta_dst2 + x, imask, dst2);
                            _mm256_maskstore_ps(delta_dst3 + x, imask, dst3);
                            _mm256_maskstore_ps(delta_dst4 + x, imask, dst4);
                        }  // for x
                    }    // for out_height
                }      // for out.depth_
            }        // for in.depth_
        }
        else
        {
            for (size_t inc = 0; inc < in.depth_;
                 ++inc, pdelta_dst_org += in_padded_area)
            {
                for (size_t outc = 0; outc < out.depth_; ++outc)
                {
                    if (!tbl.is_connected(outc, inc))
                    {
                        continue;
                    }
                    const float *pw         = &W[25 * (in.depth_ * outc + inc)];
                    const float *pdelta_src = &curr_delta[out.get_index(0, 0, outc)];
                    float *pdelta_dst       = pdelta_dst_org;
                    __m256 w0a = _mm256_and_ps(_mm256_loadu_ps(pw + 0), mask);
                    __m256 w1a = _mm256_and_ps(_mm256_loadu_ps(pw + 5), mask);
                    __m256 w2a = _mm256_and_ps(_mm256_loadu_ps(pw + 10), mask);
                    __m256 w3a = _mm256_and_ps(_mm256_loadu_ps(pw + 15), mask);
                    __m256 w4a = _mm256_maskload_ps(pw + 20, imask);
                    __m256 w0b = leftShift<4>(w0a);
                    __m256 w1b = leftShift<4>(w1a);
                    __m256 w2b = leftShift<4>(w2a);
                    __m256 w3b = leftShift<4>(w3a);
                    __m256 w4b = leftShift<4>(w4a);
                    __m256 w0c = leftShift<8>(w0a);
                    __m256 w1c = leftShift<8>(w1a);
                    __m256 w2c = leftShift<8>(w2a);
                    __m256 w3c = leftShift<8>(w3a);
                    __m256 w4c = leftShift<8>(w4a);
                    __m256 w0d = leftShift<12>(w0a);
                    __m256 w1d = leftShift<12>(w1a);
                    __m256 w2d = leftShift<12>(w2a);
                    __m256 w3d = leftShift<12>(w3a);
                    __m256 w4d = leftShift<12>(w4a);
                    size_t y   = 0;
                    do {
                        float *delta_dst0 = pdelta_dst;
                        float *delta_dst1 = &pdelta_dst[in_padded.width_ * 1];
                        float *delta_dst2 = &pdelta_dst[in_padded.width_ * 2];
                        float *delta_dst3 = &pdelta_dst[in_padded.width_ * 3];
                        float *delta_dst4 = &pdelta_dst[in_padded.width_ * 4];
                        size_t n          = 0;
                        do {
                            __m256 delta_src =
                                    _mm256_broadcast_ps((const __m128 *)(pdelta_src + n * 4));
                            __m256 dst0 = _mm256_loadu_ps(delta_dst0 + 4 * n);
                            __m256 dst1 = _mm256_loadu_ps(delta_dst1 + 4 * n);
                            __m256 dst2 = _mm256_loadu_ps(delta_dst2 + 4 * n);
                            __m256 dst3 = _mm256_loadu_ps(delta_dst3 + 4 * n);
                            __m256 dst4 = _mm256_loadu_ps(delta_dst4 + 4 * n);
                            __m256 delta_src0 =
                                    _mm256_permute_ps(delta_src, _MM_SHUFFLE(0, 0, 0, 0));
                            __m256 delta_src1 =
                                    _mm256_permute_ps(delta_src, _MM_SHUFFLE(1, 1, 1, 1));
                            __m256 delta_src2 =
                                    _mm256_permute_ps(delta_src, _MM_SHUFFLE(2, 2, 2, 2));
                            __m256 delta_src3 =
                                    _mm256_permute_ps(delta_src, _MM_SHUFFLE(3, 3, 3, 3));
                            dst0 = madd256_ps(w0a, delta_src0, dst0);
                            dst1 = madd256_ps(w1a, delta_src0, dst1);
                            dst2 = madd256_ps(w2a, delta_src0, dst2);
                            dst3 = madd256_ps(w3a, delta_src0, dst3);
                            dst4 = madd256_ps(w4a, delta_src0, dst4);
                            dst0 = madd256_ps(w0b, delta_src1, dst0);
                            dst1 = madd256_ps(w1b, delta_src1, dst1);
                            dst2 = madd256_ps(w2b, delta_src1, dst2);
                            dst3 = madd256_ps(w3b, delta_src1, dst3);
                            dst4 = madd256_ps(w4b, delta_src1, dst4);
                            dst0 = madd256_ps(w0c, delta_src2, dst0);
                            dst1 = madd256_ps(w1c, delta_src2, dst1);
                            dst2 = madd256_ps(w2c, delta_src2, dst2);
                            dst3 = madd256_ps(w3c, delta_src2, dst3);
                            dst4 = madd256_ps(w4c, delta_src2, dst4);
                            dst0 = madd256_ps(w0d, delta_src3, dst0);
                            _mm256_storeu_ps(delta_dst0 + 4 * n, dst0);
                            dst1 = madd256_ps(w1d, delta_src3, dst1);
                            _mm256_storeu_ps(delta_dst1 + 4 * n, dst1);
                            dst2 = madd256_ps(w2d, delta_src3, dst2);
                            _mm256_storeu_ps(delta_dst2 + 4 * n, dst2);
                            dst3 = madd256_ps(w3d, delta_src3, dst3);
                            _mm256_storeu_ps(delta_dst3 + 4 * n, dst3);
                            dst4 = madd256_ps(w4d, delta_src3, dst4);
                            _mm256_storeu_ps(delta_dst4 + 4 * n, dst4);
                            ++n;
                        } while (n < nblocks);
                        ++y;
                        pdelta_src += out_width;
                        pdelta_dst += h_stride2;
                    } while (y < out_height);
                }  // for out.depth_
            }    // for in.depth_
        }
    }
    else if (out_height == 1 && out_width == 1)
    {
        for (size_t inc = 0; inc < in.depth_;
             ++inc, pdelta_dst_org += in_padded_area)
        {
            __m256 sum0 = _mm256_setzero_ps();
            __m256 sum1 = _mm256_setzero_ps();
            __m256 sum2 = _mm256_setzero_ps();
            __m128 sum3 = _mm_setzero_ps();

            size_t widx  = 25 * inc;
            size_t wstep = 25 * in.depth_;
            __m256 delta_src;
            if (tbl.is_empty())
            {
                for (size_t outc = 0; outc < out.depth_; ++outc, widx += wstep)
                {
                    delta_src       = _mm256_broadcast_ss(&curr_delta[outc]);
                    const float *pw = (const float *)&W[widx];
                    __m256 w0       = _mm256_loadu_ps(pw + 0);
                    __m256 w1       = _mm256_loadu_ps(pw + 8);
                    __m256 w2       = _mm256_loadu_ps(pw + 16);
                    __m128 w3       = _mm_load_ss(pw + 24);
                    sum0            = madd256_ps(w0, delta_src, sum0);
                    sum1            = madd256_ps(w1, delta_src, sum1);
                    sum2            = madd256_ps(w2, delta_src, sum2);
                    sum3 = madd128_ss(w3, _mm256_castps256_ps128(delta_src), sum3);
                }
            }
            else
            {
                for (size_t outc = 0; outc < out.depth_; ++outc, widx += wstep)
                {
                    if (!tbl.is_connected(outc, inc))
                    {
                        continue;
                    }
                    delta_src       = _mm256_broadcast_ss(&curr_delta[outc]);
                    const float *pw = (const float *)&W[widx];
                    __m256 w0       = _mm256_loadu_ps(pw + 0);
                    __m256 w1       = _mm256_loadu_ps(pw + 8);
                    __m256 w2       = _mm256_loadu_ps(pw + 16);
                    __m128 w3       = _mm_load_ss(pw + 24);
                    sum0            = madd256_ps(w0, delta_src, sum0);
                    sum1            = madd256_ps(w1, delta_src, sum1);
                    sum2            = madd256_ps(w2, delta_src, sum2);
                    sum3 = madd128_ss(w3, _mm256_castps256_ps128(delta_src), sum3);
                }
            }
            float *delta_dst0 = pdelta_dst_org;
            float *delta_dst1 = &pdelta_dst_org[in_padded.width_ * 1];
            float *delta_dst2 = &pdelta_dst_org[in_padded.width_ * 2];
            float *delta_dst3 = &pdelta_dst_org[in_padded.width_ * 3];
            float *delta_dst4 = &pdelta_dst_org[in_padded.width_ * 4];
            __m256 dst0       = _mm256_loadu_ps(delta_dst0);
            __m256 dst1       = _mm256_loadu_ps(delta_dst1);
            __m256 dst2       = _mm256_loadu_ps(delta_dst2);
            __m256 dst3       = _mm256_loadu_ps(delta_dst3);
            __m256 dst4       = _mm256_maskload_ps(delta_dst4, imask);

            // *FROM
            // 1110 0000
            // 3222 2211
            // 4444 3333
            // ---- ---4
            //
            // *TO
            // ---0 0000
            // ---1 1111
            // ---2 2222
            // ---3 3333
            // ---4 4444
            __m256 new_sum0 =
                    _mm256_blend_ps(_mm256_setzero_ps(), sum0, 0x1F /* 0b00011111 */);
            __m256 new_sum1 =
                    _mm256_blend_ps(_mm256_setzero_ps(),
                                    _mm256_or_ps(rightShift<20>(sum0), leftShift<12>(sum1)),
                                    0x1F /* 0b00011111 */);
            __m256 new_sum2 = _mm256_blend_ps(
                        _mm256_setzero_ps(), rightShift<8>(sum1), 0x1F /* 0b00011111 */);
            __m256 new_sum3 =
                    _mm256_blend_ps(_mm256_setzero_ps(),
                                    _mm256_or_ps(rightShift<28>(sum1), leftShift<4>(sum2)),
                                    0x1F /* 0b00011111 */);
            __m256 new_sum4 =
                    _mm256_blend_ps(_mm256_setzero_ps(),
                                    _mm256_set_m128(sum3, _mm256_extractf128_ps(sum2, 1)),
                                    0x1F /* 0b00011111 */);
            dst0 = _mm256_add_ps(dst0, new_sum0);
            dst1 = _mm256_add_ps(dst1, new_sum1);
            dst2 = _mm256_add_ps(dst2, new_sum2);
            dst3 = _mm256_add_ps(dst3, new_sum3);
            dst4 = _mm256_add_ps(dst4, new_sum4);

            _mm256_maskstore_ps(delta_dst0, imask, dst0);
            _mm256_maskstore_ps(delta_dst1, imask, dst1);
            _mm256_maskstore_ps(delta_dst2, imask, dst2);
            _mm256_maskstore_ps(delta_dst3, imask, dst3);
            _mm256_maskstore_ps(delta_dst4, imask, dst4);
        }  // for
    }
    else
    {
        for (size_t inc = 0; inc < in.depth_;
             ++inc, pdelta_dst_org += in_padded_area)
        {
            for (size_t outc = 0; outc < out.depth_; ++outc)
            {
                if (!tbl.is_connected(outc, inc))
                {
                    continue;
                }

                const float *pw         = &W[25 * (in.depth_ * outc + inc)];
                const float *pdelta_src = &curr_delta[out.get_index(0, 0, outc)];
                float *pdelta_dst       = pdelta_dst_org;
                __m256 w0a              = _mm256_maskload_ps(pw + 0, imask);
                __m256 w1a              = _mm256_maskload_ps(pw + 5, imask);
                __m256 w2a              = _mm256_maskload_ps(pw + 10, imask);
                __m256 w3a              = _mm256_maskload_ps(pw + 15, imask);
                __m256 w4a              = _mm256_maskload_ps(pw + 20, imask);
                for (size_t y = 0; y < out_height;
                     ++y, pdelta_src += out_width, pdelta_dst += h_stride2)
                {
                    float *delta_dst0 = pdelta_dst;
                    float *delta_dst1 = &pdelta_dst[in_padded.width_ * 1];
                    float *delta_dst2 = &pdelta_dst[in_padded.width_ * 2];
                    float *delta_dst3 = &pdelta_dst[in_padded.width_ * 3];
                    float *delta_dst4 = &pdelta_dst[in_padded.width_ * 4];
                    for (size_t x = 0; x < out_width; ++x)
                    {
                        __m256 delta_src = _mm256_broadcast_ss(pdelta_src + x);
                        __m256 dst0      = _mm256_loadu_ps(delta_dst0);
                        __m256 dst1      = _mm256_loadu_ps(delta_dst1);
                        __m256 dst2      = _mm256_loadu_ps(delta_dst2);
                        __m256 dst3      = _mm256_loadu_ps(delta_dst3);
                        __m256 dst4      = _mm256_maskload_ps(delta_dst4, imask);
                        dst0             = madd256_ps(w0a, delta_src, dst0);
                        dst1             = madd256_ps(w1a, delta_src, dst1);
                        dst2             = madd256_ps(w2a, delta_src, dst2);
                        dst3             = madd256_ps(w3a, delta_src, dst3);
                        dst4             = madd256_ps(w4a, delta_src, dst4);
                        _mm256_storeu_ps(delta_dst0, dst0);
                        _mm256_storeu_ps(delta_dst1, dst1);
                        _mm256_storeu_ps(delta_dst2, dst2);
                        _mm256_storeu_ps(delta_dst3, dst3);
                        _mm256_maskstore_ps(delta_dst4, imask, dst4);
                        delta_dst0 += w_stride;
                        delta_dst1 += w_stride;
                        delta_dst2 += w_stride;
                        delta_dst3 += w_stride;
                        delta_dst4 += w_stride;
                    }  // for x
                }    // for y
            }      // for outc
        }        // for inc
    }

    avx_accumulate_dw(params, prev_out, curr_delta, dW, db);

    if (params.has_bias) {
        avx_accumulate_db(out, curr_delta, db);
    }
}  // avx_conv2d_5x5_back_kernel float ver

void TdConvolution::avx_op_backward(const tiny_dnn::tensor_t &prev_out,
                                    const tiny_dnn::vec_t &W,
                                    tiny_dnn::tensor_t &dW,
                                    tiny_dnn::tensor_t &db,
                                    tiny_dnn::tensor_t &curr_delta,
                                    tiny_dnn::tensor_t &prev_delta,
                                    const tiny_dnn::core::conv_params &params,
                                    int s_index, int e_index)
{
    if( params.weight.height_==5 && params.weight.width_==5 )
    {
        for( int sample=s_index ; sample<e_index ; sample++ )
        {
            avx_5x5_backward_kernel(params, prev_out[sample], W,
                                    dW[sample], db[sample],
                                    curr_delta[sample],
                                    &prev_delta[sample]);
        }
        return;
    }
    op_backward(prev_out, W, dW, db, curr_delta, prev_delta, params,
                s_index, e_index);
}
#endif
