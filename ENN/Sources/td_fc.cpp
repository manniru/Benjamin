#include "td_fc.h"

TdFC::TdFC(size_t in_dim, size_t out_dim, bool has_bias)
    : TdLayer(tiny_dnn::std_input_order(has_bias))
{
    set_params(in_dim, out_dim, has_bias);
}

size_t TdFC::fan_in_size() const
{
    return params_.in_size_;
}

size_t TdFC::fan_out_size() const
{
    return params_.out_size_;
}

std::vector<tiny_dnn::index3d<size_t> > TdFC::in_shape() const
{
    if( params_.has_bias_ )
    {
        return {tiny_dnn::index3d<size_t>(params_.in_size_, 1, 1),
                tiny_dnn::index3d<size_t>(params_.in_size_,
                                          params_.out_size_, 1),
                tiny_dnn::index3d<size_t>(params_.out_size_, 1, 1)};
    }
    else
    {
        return {tiny_dnn::index3d<size_t>(params_.in_size_, 1, 1),
                    tiny_dnn::index3d<size_t>(params_.in_size_,
                                              params_.out_size_, 1)};
    }
}

std::vector<tiny_dnn::index3d<size_t> > TdFC::out_shape() const
{
    return {tiny_dnn::index3d<size_t>(params_.out_size_, 1, 1)};
}

template <typename Allocator>
void TdFC::avx_op_forward(
        const std::vector<std::vector<float, Allocator>> &in_data,
        const std::vector<float, Allocator> &W,
        const std::vector<float, Allocator> &bias,
        std::vector<std::vector<float, Allocator>> &out_data,
        const tiny_dnn::core::fully_params &params,
        int s_index, int e_index)
{
    size_t nblocks  = params.out_size_ / 8;
    size_t nremains = params.out_size_ & 7;
    if(nremains)
    {
        int32_t mask_src[] = {
            -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0,
        };
        __m256i imask =
                _mm256_loadu_si256((__m256i const *)
                                   (mask_src + 8 - nremains));
        for( int sample=s_index ; sample<e_index ; sample++ )
        {
            const auto &in = in_data[sample];
            auto &out      = out_data[sample];
            {
                for (size_t i = 0; i < nblocks; ++i)
                {
                    __m256 b = _mm256_loadu_ps(&bias[8 * i]); // packed singles
                    _mm256_storeu_ps(&out[8 * i], b);
                }
                auto b = _mm256_maskload_ps(&bias[8 * nblocks], imask);
                _mm256_maskstore_ps(&out[8 * nblocks], imask, b);
            }
            for (size_t c = 0; c < params.in_size_; c++)
            {
                auto in_val     = _mm256_set1_ps(in[c]);
                const float *pW = &W[c * params.out_size_];
                for (size_t i = 0; i < nblocks / 2; ++i)
                {
                    __m256 sum0 = _mm256_loadu_ps(&out[16 * i]);
                    __m256 sum1 = _mm256_loadu_ps(&out[16 * i + 8]);
                    __m256 w0   = _mm256_loadu_ps(pW + 16 * i);
                    __m256 w1   = _mm256_loadu_ps(pW + 16 * i + 8);
                    sum0        = madd256_ps(w0, in_val, sum0);
                    sum1        = madd256_ps(w1, in_val, sum1);
                    _mm256_storeu_ps(&out[16 * i], sum0);
                    _mm256_storeu_ps(&out[16 * i + 8], sum1);
                }
                if (nblocks & 1)
                {
                    __m256 sum0 =
                            _mm256_loadu_ps(&out[nblocks / 2 * 16]);
                    __m256 w0 = _mm256_loadu_ps(pW + nblocks / 2 * 16);
                    sum0 = madd256_ps(w0, in_val, sum0);
                    _mm256_storeu_ps(&out[nblocks / 2 * 16], sum0);
                }
                __m256 sum = _mm256_maskload_ps(&out[8 * nblocks],
                        imask);
                __m256 w = _mm256_maskload_ps(pW + 8 * nblocks, imask);
                sum = madd256_ps(w, in_val, sum);
                _mm256_maskstore_ps(&out[8 * nblocks], imask, sum);
            }
        }
    }
    else
    {
        for( int sample=s_index ; sample<e_index ; sample++ )
        {
            const auto &in = in_data[sample];
            auto &out      = out_data[sample];
            for (size_t i = 0; i < nblocks; ++i)
            {
                __m256 b = _mm256_loadu_ps(&bias[8 * i]);
                _mm256_storeu_ps(&out[8 * i], b);
            }
            for (size_t c = 0; c < params.in_size_; c++)
            {
                auto in_val     = _mm256_set1_ps(in[c]);
                const float *pW = &W[c*params.out_size_];
                for (size_t i = 0; i < nblocks; ++i)
                {
                    __m256 sum = _mm256_loadu_ps(&out[8 * i]);
                    __m256 w   = _mm256_loadu_ps(pW + 8 * i);
                    sum        = madd256_ps(w, in_val, sum);
                    _mm256_storeu_ps(&out[8 * i], sum);
                }
            }
        }
    }
}

template <typename Allocator>
void TdFC::avx_op_backward(
        const std::vector<std::vector<float, Allocator>> &prev_out,
        const std::vector<float, Allocator> &W,
        std::vector<std::vector<float, Allocator>> &dW,
        std::vector<std::vector<float, Allocator>> &db,
        std::vector<std::vector<float, Allocator>> &curr_delta,
        std::vector<std::vector<float, Allocator>> &prev_delta,
        const tiny_dnn::core::fully_params &params, int s_index,
        int e_index)
{
    for( int sample=s_index ; sample<e_index ; sample++ )
    {
        auto &prev_delta2 = prev_delta[sample];
        auto &curr_delta2 = curr_delta[sample];
        auto &prev_out2   = prev_out[sample];
        auto &dW2         = dW[sample];
        auto &db2         = db[sample];
        for( size_t c=0 ; c<params.in_size_ ; c++ )
        {
            // propagate delta to previous layer
            // prev_delta[c] += current_delta[r] * W_[c*out_size_+r]
            prev_delta2[c] += vectorize::dot(
                        &curr_delta2[0], &W[c * params.out_size_],
                        params.out_size_);
        }
        int len = params.out_size_;

        for( size_t c=0 ; c<params.in_size_ ; c++ )
        {
            vectorize::muladd(&curr_delta2[0], prev_out2[c],
                    len, &dW2[c*params.out_size_]);
        }
        // vec_t& db = *in_grad[2];
        vectorize::reduce(&curr_delta2[0], len, &db2[0]);
    }
}

void TdFC::op_forward(const tiny_dnn::tensor_t &in_data,
                const tiny_dnn::vec_t &W,
                const tiny_dnn::vec_t &bias,
                tiny_dnn::tensor_t &out_data,
                const tiny_dnn::core::fully_params &params,
                int s_index, int e_index)
{
    for( int i=s_index ; i<e_index ; i++ )
    {
        const tiny_dnn::vec_t &in = in_data[i];
        tiny_dnn::vec_t &out      = out_data[i];

        for( size_t i=0 ; i<params.out_size_ ; i++ )
        {
            out[i] = float_t{0};
            for( size_t c=0 ; c<params.in_size_ ; c++ )
            {
                out[i] += W[c * params.out_size_ + i] * in[c];
            }

            if( params.has_bias_ )
            {
                out[i] += bias[i];
            }
        }
    };
}

void TdFC::op_backward(const tiny_dnn::tensor_t &prev_out,
                       const tiny_dnn::vec_t &W,
                       tiny_dnn::tensor_t &dW,
                       tiny_dnn::tensor_t &db,
                       tiny_dnn::tensor_t &curr_delta,
                       tiny_dnn::tensor_t &prev_delta,
                       const tiny_dnn::core::fully_params &params,
                       int s_index, int e_index)
{
    for( int sample=s_index ; sample<e_index ; sample++ )
    {
        for( size_t c=0 ; c<params.in_size_ ; c++ )
        {
            // propagate delta to previous layer
            // prev_delta[c] += current_delta[r] * W_[c*out_size_+r]
            prev_delta[sample][c] += vectorize::dot(
                &curr_delta[sample][0], &W[c * params.out_size_],
                    params.out_size_);
        }

        int len = params.out_size_;
        // accumulate weight-step using delta
        // dW[c * out_size + i] += current_delta[i] * prev_out[c]
        for( size_t c=0 ; c<params.in_size_ ; c++ )
        {
            vectorize::muladd(&curr_delta[sample][0],
                    prev_out[sample][c], len,
                    &dW[sample][c * params.out_size_]);
        }

        if( params.has_bias_ )
        {
            // vec_t& db = *in_grad[2];
            for( int i=0 ; i<len ; i++ )
            {
                db[sample][i] += curr_delta[sample][i];
            }
        }
    }
}

void TdFC::forward(int s_index, int e_index)
{
    // launch fully connected kernel
    auto params = params_.fully();

    // incomimg/outcoming data
    tiny_dnn::tensor_t &in_data_0   = in_edges[0]->data_;
    tiny_dnn::tensor_t &W           = in_edges[1]->data_;
    tiny_dnn::tensor_t &bias        = in_edges[2]->data_;
    tiny_dnn::tensor_t &out_data_0  = out_edges->data_;

    // initialize outputs
    tiny_dnn::fill_tensor(out_data_0, float_t{0});

#ifdef CNN_USE_AVX
    avx_op_forward(in_data_0, W[0], bias[0],
            out_data_0, params, s_index, e_index);
#else
    op_forward(in_data_0, W[0], bias[0], out_data_0, params, s_index,
            e_index);
#endif
}

void TdFC::backward(int s_index, int e_index)
{
    // launch fully connected kernel
    auto params = params_.fully();

    // incoming/outcoming data
    tiny_dnn::tensor_t &prev_out = in_edges[0]->data_;
    tiny_dnn::tensor_t &W        = in_edges[1]->data_;
    tiny_dnn::tensor_t &dW       = in_edges[1]->grad_;
    tiny_dnn::tensor_t &db       = in_edges[2]->grad_;
    tiny_dnn::tensor_t &prev_delta = in_edges[0]->grad_;
    tiny_dnn::tensor_t &curr_delta = out_edges->grad_;

    // initialize outputs
    fill_tensor(prev_delta, float_t{0});

#ifdef CNN_USE_AVX
    avx_op_backward( prev_out, W[0], dW, db, curr_delta,
            prev_delta, params, s_index, e_index);
#else
    op_backward(prev_out, W[0], dW, db, curr_delta,
            prev_delta, params, s_index, e_index);
#endif
}

std::string TdFC::layer_type() const
{
    return "fully-connected";
}

void TdFC::set_params(const size_t in_size,
                      const size_t out_size, bool has_bias)
{
    params_.in_size_  = in_size;
    params_.out_size_ = out_size;
    params_.has_bias_ = has_bias;
}

