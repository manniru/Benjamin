#include "td_fc.h"

TdFC::TdFC(size_t in_dim, size_t out_dim, bool has_bias,
           tiny_dnn::core::backend_t bt)
    : TdLayer(tiny_dnn::std_input_order(has_bias),
      {tiny_dnn::vector_type::data})
{
    set_params(in_dim, out_dim, has_bias);
    init_backend(backend_type);
    backend_type = bt;
}

TdFC::TdFC(TdFC &&other)
    : TdLayer(std::move(other)),
      params_(std::move(other.params_)),
      kernel_fwd_(std::move(other.kernel_fwd_)),
      kernel_back_(std::move(other.kernel_back_))
{
    init_backend(std::move(other.getEngine()));
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
                    tiny_dnn::index3d<size_t>(params_.in_size_, params_.out_size_, 1),
                    tiny_dnn::index3d<size_t>(params_.out_size_, 1, 1)};
    }
    else
    {
        return {tiny_dnn::index3d<size_t>(params_.in_size_, 1, 1),
                    tiny_dnn::index3d<size_t>(params_.in_size_, params_.out_size_, 1)};
    }
}

std::vector<tiny_dnn::index3d<size_t> > TdFC::out_shape() const
{
    return {tiny_dnn::index3d<size_t>(params_.out_size_, 1, 1)};
}

void TdFC::forward_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data, std::vector<tiny_dnn::tensor_t *> &out_data)
{
    // forward fully connected op context
    fwd_ctx_.set_in_out(in_data, out_data);
    fwd_ctx_.setParallelize(parallelized);
    fwd_ctx_.setEngine(getEngine());

    // launch fully connected kernel
    kernel_fwd_->compute(fwd_ctx_);
}

void TdFC::back_propagation(const std::vector<tiny_dnn::tensor_t *> &in_data,
                            const std::vector<tiny_dnn::tensor_t *> &out_data,
                            std::vector<tiny_dnn::tensor_t *> &out_grad,
                            std::vector<tiny_dnn::tensor_t *> &in_grad)
{
    // backward fully connected op context
    bwd_ctx_.set_in_out(in_data, out_data, out_grad, in_grad);
    bwd_ctx_.setParallelize(parallelized);
    bwd_ctx_.setEngine(getEngine());

    // launch fully connected kernel
    kernel_back_->compute(bwd_ctx_);
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

void TdFC::init_backend(tiny_dnn::core::backend_t backend_type)
{
    tiny_dnn::core::OpKernelConstruction ctx =
            tiny_dnn::core::OpKernelConstruction(&params_);

    kernel_fwd_.reset(new tiny_dnn::FullyConnectedOp(ctx));
}
