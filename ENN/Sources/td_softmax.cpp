#include "td_softmax.h"

TdSoftmax::TdSoftmax() : TdLayer({tiny_dnn::vector_type::data},
                            {tiny_dnn::vector_type::data})
{
    in_shape_ = tiny_dnn::shape3d(0, 0, 0);
    trainable = false;
}

std::string TdSoftmax::layer_type() const
{
    return "softmax-activation";
}

void TdSoftmax::forward_activation(const tiny_dnn::vec_t &x,
                                   tiny_dnn::vec_t &y)
{
    const float_t alpha = *std::max_element(x.begin(), x.end());
    float_t denominator(0);
    for( size_t j=0 ; j<x.size() ; j++ )
    {
        y[j] = std::exp(x[j] - alpha);
        denominator += y[j];
    }
    for( size_t j=0 ; j<x.size() ; j++ )
    {
        y[j] /= denominator;
    }
}

void TdSoftmax::backward_activation(const tiny_dnn::vec_t &x,
                                    const tiny_dnn::vec_t &y,
                                    tiny_dnn::vec_t &dx,
                                    const tiny_dnn::vec_t &dy)
{
    const size_t len = dy.size();

    // auxilliary vector to store element wise softmax
    // gradients of all elements

    tiny_dnn::vec_t df(len, 0);
    for( size_t j=0 ; j<x.size() ; j++ )
    {
        for( size_t k=0 ; k<x.size() ; k++ )
        {
            if( k==j )
            {
                df[k] = y[j] * (float_t(1) - y[j]);
            }
            else
            {
                df[k] = -y[k] * y[j];
            }
        }
        // dx = dy * (gradient of softmax)
        dx[j] = vectorize::dot(&dy[0], &df[0], len);
    }
}

void TdSoftmax::set_in_shape(const tiny_dnn::shape3d &in_shape)
{
    this->in_shape_ = in_shape;
}

std::vector<tiny_dnn::shape3d> TdSoftmax::in_shape() const
{
    return {in_shape_};
}

std::vector<tiny_dnn::shape3d> TdSoftmax::out_shape() const
{
    return {in_shape_};
}

std::pair<float_t, float_t> TdSoftmax::scale() const
{
    return std::make_pair(float_t(0), float_t(1));
}

void TdSoftmax::forward_propagation(
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        std::vector<tiny_dnn::tensor_t *> &out_data)
{
    const tiny_dnn::tensor_t &x = *in_data[0];
    tiny_dnn::tensor_t &y       = *out_data[0];
    tiny_dnn::for_i(x.size(), [&](size_t i)
    {
        forward_activation(x[i], y[i]);
    });
}

void TdSoftmax::back_propagation(
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        const std::vector<tiny_dnn::tensor_t *> &out_data,
        std::vector<tiny_dnn::tensor_t *> &out_grad,
        std::vector<tiny_dnn::tensor_t *> &in_grad)
{
    const tiny_dnn::tensor_t &x  = *in_data[0];
    const tiny_dnn::tensor_t &y  = *out_data[0];
    const tiny_dnn::tensor_t &dy = *out_grad[0];
    tiny_dnn::tensor_t &dx       = *in_grad[0];

    int len = x.size();
    for( int i=0 ; i<len ; i++ )
    {
        backward_activation(x[i], y[i], dx[i], dy[i]);
    };
}
