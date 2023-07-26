#include "td_leaky_relu.h"

TdLeakyRelu::TdLeakyRelu(const float_t epsilon)
    : TdLayer({tiny_dnn::vector_type::data},
                        {tiny_dnn::vector_type::data})
{
    epsilon_ = epsilon;
    in_shape_ = tiny_dnn::shape3d(0, 0, 0);
}


std::vector<tiny_dnn::shape3d> TdLeakyRelu::in_shape() const
{
    return {in_shape_};
}

std::vector<tiny_dnn::shape3d> TdLeakyRelu::out_shape() const
{
    return {in_shape_};
}

void TdLeakyRelu::set_in_shape(const tiny_dnn::shape3d &in_shape)
{
    this->in_shape_ = in_shape;
}

void TdLeakyRelu::forward_propagation(
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

void TdLeakyRelu::back_propagation(
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        const std::vector<tiny_dnn::tensor_t *> &out_data,
        std::vector<tiny_dnn::tensor_t *> &out_grad,
        std::vector<tiny_dnn::tensor_t *> &in_grad)
{
    tiny_dnn::tensor_t &dx       = *in_grad[0];
    const tiny_dnn::tensor_t &dy = *out_grad[0];
    const tiny_dnn::tensor_t &x  = *in_data[0];
    const tiny_dnn::tensor_t &y  = *out_data[0];
    tiny_dnn::for_i(x.size(), [&](size_t i)
    {
        backward_activation(x[i], y[i], dx[i], dy[i]);
    });
}

std::string TdLeakyRelu::layer_type() const
{
    return "leaky-relu-activation";
}

void TdLeakyRelu::forward_activation(const tiny_dnn::vec_t &x,
                                     tiny_dnn::vec_t &y)
{
    for( size_t j=0 ; j<x.size() ; j++ )
    {
        if( x[j] > float_t(0) )
        {
            y[j] = x[j];
        }
        else
        {
            y[j] = epsilon_ * x[j];
        }
    }
}

void TdLeakyRelu::backward_activation(const tiny_dnn::vec_t &x,
                                      const tiny_dnn::vec_t &y,
                                      tiny_dnn::vec_t &dx,
                                      const tiny_dnn::vec_t &dy)
{
    for( size_t j=0 ; j<x.size() ; j++ )
    {
        // dx = dy * (gradient of leaky relu)
        dx[j] = dy[j] * (y[j] > float_t(0) ? float_t(1) : epsilon_);
    }
}

std::pair<float_t, float_t> TdLeakyRelu::scale() const
{
    return std::make_pair(float_t(0.1), float_t(0.9));
}

