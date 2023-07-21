#ifndef TDSOFTMAX_H
#define TDSOFTMAX_H

#include <string>
#include <utility>

#include "tiny_dnn/activations/activation_layer.h"
#include "tiny_dnn/layers/layer.h"

class TdSoftmax : public tiny_dnn::activation_layer
{
public:
    std::string layer_type() const override;

    void forward_activation(const tiny_dnn::vec_t &x,
                            tiny_dnn::vec_t &y) override;

    void backward_activation(const tiny_dnn::vec_t &x,
                             const tiny_dnn::vec_t &y,
                             tiny_dnn::vec_t &dx,
                             const tiny_dnn::vec_t &dy) override;

    std::pair<float_t, float_t> scale() const override;
};

#endif // TDSOFTMAX_H
