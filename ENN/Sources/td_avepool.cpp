#include "td_avepool.h"

TdAvePool::TdAvePool(size_t in_width, size_t in_height,
                     size_t in_channels, size_t pool_size_x,
                     size_t pool_size_y, size_t stride_x,
                     size_t stride_y, bool ceil_mode,
                     tiny_dnn::padding pad_type)
    : tiny_dnn::partial_connected_layer(
          in_width * in_height * in_channels,
          pool_out_length(in_width, pool_size_x, stride_x,
                          ceil_mode, pad_type) *
          pool_out_length(in_height, pool_size_y, stride_y,
                          ceil_mode, pad_type) *
          in_channels, in_channels, in_channels,
          float_t(1) / (pool_size_x * pool_size_y)),
      stride_x_(stride_x),
      stride_y_(stride_y),
      pool_size_x_(pool_size_x),
      pool_size_y_(pool_size_y),
      pad_type_(pad_type),
      ceil_mode_(ceil_mode),
      in_(in_width, in_height, in_channels),
      out_( pool_out_length(in_width, pool_size_x, stride_x,
                          ceil_mode, pad_type),
          pool_out_length(in_height, pool_size_y, stride_y,
                          ceil_mode, pad_type),
          in_channels),
      w_(pool_size_x, pool_size_y, in_channels)
{
    if( (in_width%pool_size_x) || (in_height%pool_size_y) )
    {
//      pooling_size_mismatch(in_width, in_height,
//        pool_size_x, pool_size_y);
    }

    init_connection(pool_size_x, pool_size_y);
}

std::vector<tiny_dnn::index3d<size_t> > TdAvePool::in_shape() const
{
    return {in_, w_, tiny_dnn::index3d<size_t>(1, 1, out_.depth_)};
}

std::vector<tiny_dnn::index3d<size_t> > TdAvePool::out_shape() const
{
    return {out_};
}

std::string TdAvePool::layer_type() const
{
    return "ave-pool";
}

void TdAvePool::forward_propagation(
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        std::vector<tiny_dnn::tensor_t *> &out_data)
{
    tiny_average_pooling_kernel(parallelize_, in_data, out_data, out_,
                tiny_dnn::partial_connected_layer::scale_factor_,
                tiny_dnn::partial_connected_layer::out2wi_);
}

void TdAvePool::back_propagation(
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        const std::vector<tiny_dnn::tensor_t *> &out_data,
        std::vector<tiny_dnn::tensor_t *> &out_grad,
        std::vector<tiny_dnn::tensor_t *> &in_grad)
{
    tiny_average_pooling_back_kernel(
                parallelize_, in_data, out_data, out_grad,
                in_grad, in_,
                tiny_dnn::partial_connected_layer::scale_factor_,
                tiny_dnn::partial_connected_layer::weight2io_,
                tiny_dnn::partial_connected_layer::in2wo_,
                tiny_dnn::partial_connected_layer::bias2out_);
}

std::pair<size_t, size_t> TdAvePool::pool_size() const
{
    return std::make_pair(pool_size_x_, pool_size_y_);
}

size_t TdAvePool::pool_out_dim(size_t in_size, size_t pooling_size,
                               size_t stride)
{
    return static_cast<int>(
                std::ceil((static_cast<float_t>(in_size) -
                           pooling_size) / stride) + 1);
}

void TdAvePool::init_connection(size_t pooling_size_x,
                                size_t pooling_size_y)
{
    for( size_t c=0 ; c<in_.depth_ ; ++c )
    {
        for( size_t y=0 ; y<in_.height_-pooling_size_y+1 ;
             y += stride_y_ )
        {
            for( size_t x=0 ; x<in_.width_-pooling_size_x+1 ;
                 x+=stride_x_ )
            {
                connect_kernel(pooling_size_x,
                               pooling_size_y, x, y, c);
            }
        }
    }

    for( size_t c=0 ; c<in_.depth_ ; ++c )
    {
        for( size_t y=0 ; y<out_.height_ ; ++y )
        {
            for( size_t x=0 ; x<out_.width_ ; ++x )
            {
                this->connect_bias(c, out_.get_index(x, y, c));
            }
        }
    }
}

void TdAvePool::connect_kernel(size_t pooling_size_x,
                               size_t pooling_size_y, size_t x, size_t y,
                               size_t inc)
{
    size_t dymax  = std::min(pooling_size_y, in_.height_ - y);
    size_t dxmax  = std::min(pooling_size_x, in_.width_ - x);
    size_t dstx   = x / stride_x_;
    size_t dsty   = y / stride_y_;
    size_t outidx = out_.get_index(dstx, dsty, inc);
    for( size_t dy=0 ; dy<dymax ; ++dy )
    {
        for( size_t dx=0 ; dx<dxmax; ++dx )
        {
            this->connect_weight(in_.get_index(x + dx, y + dy, inc),
                                 outidx, inc);
        }
    }
}

void tiny_average_pooling_kernel(bool parallelize,
            const std::vector<tiny_dnn::tensor_t *> &in_data,
            std::vector<tiny_dnn::tensor_t *> &out_data,
            const tiny_dnn::shape3d &out_dim, float_t scale_factor,
            std::vector<tiny_dnn::partial_connected_layer::
                                 wi_connections> &out2wi)
{
    tiny_dnn::for_i(parallelize, in_data[0]->size(), [&](size_t sample)
    {
        const tiny_dnn::vec_t &in = (*in_data[0])[sample];
        const tiny_dnn::vec_t &W  = (*in_data[1])[0];
        const tiny_dnn::vec_t &b  = (*in_data[2])[0];
        tiny_dnn::vec_t &out      = (*out_data[0])[sample];

        auto oarea = out_dim.area();
        size_t idx = 0;
        for( size_t d=0; d<out_dim.depth_ ; ++d )
        {
            float_t weight = W[d] * scale_factor;
            float_t bias   = b[d];
            for( size_t i=0 ; i<oarea ; ++i, ++idx )
            {
                const auto &connections = out2wi[idx];
                float_t value{0};
                for( auto connection:connections )
                {
                    value += in[connection.second];
                }
                value *= weight;
                value += bias;
                out[idx] = value;
            }
        }

        assert(out.size() == out2wi.size());
    });
}

void tiny_average_pooling_back_kernel(bool parallelize,
        const std::vector<tiny_dnn::tensor_t *> &in_data,
        const std::vector<tiny_dnn::tensor_t *> &out_data,
        std::vector<tiny_dnn::tensor_t *> &out_grad,
        std::vector<tiny_dnn::tensor_t *> &in_grad,
        const tiny_dnn::shape3d &in_dim, float_t scale_factor,
        std::vector<tiny_dnn::partial_connected_layer::
                                      io_connections> &weight2io,
        std::vector<tiny_dnn::partial_connected_layer::
                                      wo_connections> &in2wo,
        std::vector<std::vector<size_t> > &bias2out)
{
    CNN_UNREFERENCED_PARAMETER(out_data);
    tiny_dnn::for_i(parallelize, in_data[0]->size(), [&](size_t sample)
    {
        const tiny_dnn::vec_t &prev_out = (*in_data[0])[sample];
        const tiny_dnn::vec_t &W        = (*in_data[1])[0];
        tiny_dnn::vec_t &dW             = (*in_grad[1])[sample];
        tiny_dnn::vec_t &db             = (*in_grad[2])[sample];
        tiny_dnn::vec_t &prev_delta     = (*in_grad[0])[sample];
        tiny_dnn::vec_t &curr_delta     = (*out_grad[0])[sample];

        auto inarea = in_dim.area();
        size_t idx  = 0;
        for( size_t i=0 ; i<in_dim.depth_ ; ++i )
        {
            float_t weight = W[i] * scale_factor;
            for ( size_t j=0 ; j<inarea ; ++j, ++idx )
            {
                prev_delta[idx] = weight *
                        curr_delta[in2wo[idx][0].second];
            }
        }

        for( size_t i=0 ; i<weight2io.size() ; ++i )
        {
            const auto &connections = weight2io[i];
            float_t diff{0};

            for( auto connection:connections )
                diff += prev_out[connection.first] *
                        curr_delta[connection.second];

            dW[i] += diff * scale_factor;
        }

        for( size_t i=0 ; i<bias2out.size() ; i++ )
        {

            for( size_t i=0 ; i<bias2out.size() ; i++ )
            {
                const std::vector<size_t> &outs = bias2out[i];
                float_t diff{0};

                for( auto o:outs )
                {
                    diff += curr_delta[o];
                }

                db[i] += diff;
            }
        }
    });
}
