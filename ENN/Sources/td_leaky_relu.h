#ifndef TD_LEAKY_RELU_H
#define TD_LEAKY_RELU_H

#include <string>
#include <utility>

#include "tiny_dnn/activations/activation_layer.h"
#include "tiny_dnn/layers/layer.h"

class TdLeakyRelu : public tiny_dnn::activation_layer
{
public:
    explicit TdLeakyRelu(const float_t epsilon = 0.01);

    std::string layer_type() const override;

    void forward_activation(const tiny_dnn::vec_t &x,
                            tiny_dnn::vec_t &y) override;

    void backward_activation(const tiny_dnn::vec_t &x,
                             const tiny_dnn::vec_t &y,
                             tiny_dnn::vec_t &dx,
                             const tiny_dnn::vec_t &dy) override;

    std::pair<float_t, float_t> scale() const override;

    float_t epsilon_;
};

#endif // TD_LEAKY_RELU_H
