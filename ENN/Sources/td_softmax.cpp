#include "td_softmax.h"

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

std::pair<float_t, float_t> TdSoftmax::scale() const
{
    return std::make_pair(float_t(0), float_t(1));
}
