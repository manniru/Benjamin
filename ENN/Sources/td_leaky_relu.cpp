#include "td_leaky_relu.h"

TdLeakyRelu::TdLeakyRelu(const float_t epsilon)
    : TdLayer({tiny_dnn::vector_type::data})
{
    epsilon_ = epsilon;
    in_shape_ = tiny_dnn::shape3d(0, 0, 0);
    trainable = false;
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

void TdLeakyRelu::forward(int s_index, int e_index)
{
    tiny_dnn::tensor_t &x = in_edges[0]->data_;
    tiny_dnn::tensor_t &y = out_edges->data_;
    for( int i=s_index ; i<e_index ; i++ )
    {
        forward_activation(x[i], y[i]);
    }
}

void TdLeakyRelu::back_propagation(
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        tiny_dnn::tensor_t *out_data,
        tiny_dnn::tensor_t *out_grad,
        std::vector<tiny_dnn::tensor_t *> &in_grad, int s_index,
        int e_index)
{
    tiny_dnn::tensor_t &dx       = *in_grad[0];
    const tiny_dnn::tensor_t &x  = *in_data[0];
    tiny_dnn::tensor_t &dy = *out_grad;
    tiny_dnn::tensor_t &y  = *out_data;
    for( int i=s_index ; i<e_index ; i++ )
    {
        backward_activation(x[i], y[i], dx[i], dy[i]);
    }
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
        if( y[j]>0 )
        {
            dx[j] = dy[j];
        }
        else
        {
            dx[j] = dy[j] * epsilon_;
        }
    }
}

std::pair<float_t, float_t> TdLeakyRelu::scale() const
{
    return std::make_pair(float_t(0.1), float_t(0.9));
}

