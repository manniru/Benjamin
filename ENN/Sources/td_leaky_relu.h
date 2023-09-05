#ifndef TD_LEAKY_RELU_H
#define TD_LEAKY_RELU_H

#include <string>
#include <utility>
#include <vector>

#include "td_layer.h"
#include "tiny_dnn/util/util.h"

class TdLeakyRelu : public TdLayer
{
public:
    explicit TdLeakyRelu(const float_t epsilon = 0.01);
    std::vector<tiny_dnn::shape3d> in_shape() const override;
    std::vector<tiny_dnn::shape3d> out_shape() const override;
    void set_in_shape(const tiny_dnn::shape3d &in_shape) override;
    void forward_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
            tiny_dnn::tensor_t *out_data,
            int s_index, int e_index) override;
    void back_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
            tiny_dnn::tensor_t *out_data,
            tiny_dnn::tensor_t *out_grad,
            std::vector<tiny_dnn::tensor_t *> &in_grad,
            int s_index, int e_index) override;
    std::string layer_type() const override;
    void forward_activation(const tiny_dnn::vec_t &x,
                            tiny_dnn::vec_t &y);
    void backward_activation(const tiny_dnn::vec_t &x,
                             const tiny_dnn::vec_t &y,
                             tiny_dnn::vec_t &dx,
                             const tiny_dnn::vec_t &dy);
    std::pair<float_t, float_t> scale() const;

    float_t epsilon_;
private:
    tiny_dnn::shape3d in_shape_;
};

#endif // TD_LEAKY_RELU_H
