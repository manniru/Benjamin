#include "td_leaky_relu.h"


TdLeakyRelu::TdLeakyRelu(const float_t epsilon)
    : tiny_dnn::activation_layer(tiny_dnn::shape3d(0, 0, 0)),
      epsilon_(epsilon)
{

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

