#include "td_layer.h"
#include <QString>

TdLayer::TdLayer(std::vector<tiny_dnn::vector_type> in_t)
{
    in_edges.resize(in_t.size());
    initialized = false;
    parallelized = true;

    in_channels = in_t.size();
    fwd_in_data.resize(in_channels);
    bwd_in_data.resize(in_channels);
    bwd_in_grad.resize(in_channels);

    in_type = in_t;
    weight_init = new tiny_dnn::weight_init::xavier;
    bias_init   = new tiny_dnn::weight_init::constant;
    trainable   = true;
}

void TdLayer::setParallelize(bool parallelize)
{
    parallelized = parallelize;
}

std::string TdLayer::kernel_file() const
{
    return std::string("empty_kernel_str");
}

std::string TdLayer::kernel_header() const
{
    return std::string();
}

size_t TdLayer::inDataSize() const
{
    return sumif(in_shape(),
                 [&](size_t i)
    {  // NOLINT
        return in_type[i] == tiny_dnn::vector_type::data;
    }, [](const tiny_dnn::shape3d &s)
    {
    return s.size();
});
}

size_t TdLayer::outDataSize() const
{
    size_t sum = 0;
    for( size_t i=0 ; i<out_shape().size() ; i++ )
    {
        sum += out_shape()[i].size();
    }
    return sum;
}

void TdLayer::setOutGrads(std::vector<tiny_dnn::tensor_t> &grad,
                          int s_index, int e_index)
{
    tiny_dnn::tensor_t &dst_grad = out_edges->grad_;
    for( int i=s_index ; i<e_index ; i++ )
    {
        dst_grad[i] = grad[i][0];
    }
}

void TdLayer::setInData(const std::vector<tiny_dnn::tensor_t> &data,
                        int batch_size, int offset)
{
    for( size_t ch=0 ; ch<in_channels ; ch++ )
    {
        if( in_type[ch]!=tiny_dnn::vector_type::data )
        {
            continue;
        }
        tiny_dnn::tensor_t &dst_data = in_edges[ch]->data_;
        size_t in_size     = in_edges[ch]->shape_.size();
        dst_data.resize(batch_size);

        for( int i=offset ; i<(offset+batch_size) ; i++ )
        {
            if( data[i][ch].size()!=in_size )
            {
                qDebug() << "Data Size and Layer Size Not Matched!"
                         << data[i][ch].size() << in_size;
                exit(1);
            }
            dst_data[i-offset] = data[i][ch];
        }
    }
}

void TdLayer::setInData(tiny_dnn::vec_t &data)
{
    // 0 -> ch
    tiny_dnn::tensor_t &dst_data = in_edges[0]->data_;
    size_t in_size     = in_edges[0]->shape_.size();
    size_t sample_size = 1;
    dst_data.resize(sample_size);

    if( data.size()!=in_size )
    {
        qDebug() << "Data Size and Layer Size Not Matched!"
                 << data.size() << in_size;
        exit(1);
    }
    dst_data[0] = data;
}

/**
 * return output value range
 * used only for calculating target value from label-id in final(output)
 *layer
 * override properly if the layer is intended to be used as output layer
 **/
std::pair<float_t, float_t> TdLayer::out_value_range() const
{
    return {float_t{0.0}, float_t{1.0}};
}

/**
 * set input shape of a layer (only used internally while shape inferring)
 */
void TdLayer::set_in_shape(const tiny_dnn::shape3d &in_shape) {
    CNN_UNREFERENCED_PARAMETER(in_shape);
    qDebug() << "Can't set shape. Shape inferring not applicable for this "
                "layer (yet).";
}

/**
 * number of incoming connections for each output unit
 * used only for weight/bias initialization methods which require fan-in
 *size
 *(e.g. xavier)
 * override if the layer has trainable weights, and scale of initialization
 *is
 *important
 **/
size_t TdLayer::fan_in_size() const
{
    return in_shape()[0].width_;
}

// override to allow initialization of multiple size weight matrices.
size_t TdLayer::fan_in_size(size_t) const
{
    return fan_in_size();  // fallback to single weight matrix.
}

/**
 * number of outgoing connections for each input unit
 * used only for weight/bias initialization methods which require fan-out
 *size
 *(e.g. xavier)
 * override if the layer has trainable weights, and scale of initialization
 *is
 *important
 **/
size_t TdLayer::fan_out_size() const
{
    return out_shape()[0].width_;
}

// override to allow initialization of multiple size weight vectors.
size_t TdLayer::fan_out_size(size_t) const
{
    return fan_out_size();  // fallback to single weight matrix
}

void TdLayer::backward(int s_index, int e_index)
{
    // organize input/output vectors from storage
    for( size_t i = 0; i < in_channels; i++ )
    {
        const auto &nd = in_edges[i];
        bwd_in_data[i] = &nd->data_;
        bwd_in_grad[i] = &(nd->grad_);
    }
    bwd_out_data = &out_edges->data_;
    bwd_out_grad = &out_edges->grad_;
    back_propagation(bwd_in_data, bwd_out_data, bwd_out_grad,
                     bwd_in_grad, s_index, e_index);
}

/* @brief Allocates data in the computational graph and reset weights if
 * it's needed or the data is not already initialized.
 *
 * @param reset_weight Boolean value to force to reset the weights.
 * Weights will be automatically reset if the are not initialized.
 *
 */
void TdLayer::setup(bool reset_weight)
{
    /* The input shape (width x height x depth) must be equal to the number
      * of input channels a.k.a the number of incoming vectors or 'edges' in
      * the computational nomenclature. Same is applied to output shape and
      * numbers of output edges.*/
    if( in_shape().size()!=in_channels ||
            out_shape().size()!=1 ) // 1=out_channels
    {
        qDebug() << "Connection mismatch at setup layer";
        exit(0);
    }

    // An 'edge' is created in the computational graph from the current
    // layer/node to each output node and allocates the needed memory.
    // The number of output nodes is determined by the layer interface.
    // In order to handle graph based networks, which a layer/node might
    // have multiple input/output connections, we need to check that the
    // connection edge does not already exists if we don't want duplicated
    // memory allocation.
    if( !out_edges )
    {
        // connection edge doesn't exist, so we proceed to allocate the
        // necessary memory.
        out_edges = new TdEdge(this, out_shape()[0],
                              tiny_dnn::vector_type::data);
    }

    // reset the weights if necessary, or in case that the data is
    // still not initialized.
    if( reset_weight || !initialized )
    {
        initWeight();
    }
}

/* @brief Initializes the vectors containing the trainable data.
 *
 * In case that a layer/node is set to be not trainable, it does
 * nothing and returns a void. Otherwise, for each input connection
 * and depending of the data nature (weight or bias) calls their
 * pertinent initialization function and fill the vectors with the
 * data generated by the mentioned functions.
 *
 */
void TdLayer::initWeight()
{
    // layer/node is not trainable, do nothing and mark the layer/node
    // as initialized.
    if( !trainable )
    {
        initialized = true;
        return;
    }

    // Fill vector values with data generated by the initialization
    // function. The pointer to the data is obtained from the
    // computational graph and the methods fan_in_size() and fan_out_size()
    // return the number of incoming/outcoming connections for each
    // input/output unit.
    for( size_t i=0 ; i<in_channels ; i++ )
    {
        if( !in_edges[i] )
        {
            in_edges[i] = new TdEdge(nullptr, in_shape()[i], in_type[i]);
        }
        switch( in_type[i] )
        {
        // fill vectors of weight type
        case tiny_dnn::vector_type::weight:
            weight_init->fill(&(in_edges[i]->data_)[0], fan_in_size(i),
                              fan_out_size(i));
            break;
            // fill vector of bias type
        case tiny_dnn::vector_type::bias:
            bias_init->fill(&(in_edges[i]->data_)[0], fan_in_size(i),
                    fan_out_size(i));
            break;
        default: break;
        }
    }
    // in case we succeed with data initialization, we mark the
    // layer/node as initialized.
    initialized = true;
}

void TdLayer::clearGrads(int batch_size)
{
    for( size_t i=0 ; i<in_type.size() ; i++ )
    {
        in_edges[i]->clear_grads(0, batch_size);
    }
}

void TdLayer::updateWeight(tiny_dnn::optimizer *o, int batch_size)
{
    for( size_t i=0; i<in_type.size() ; i++ )
    {
        if( trainable && is_trainable_weight(in_type[i]) )
        {
            tiny_dnn::vec_t &target = in_edges[i]->data_[0];
            in_edges[i]->merge_grads(&weights_diff);
            float_t rcp_batch_size =
                    float_t(1.0) / float_t((&in_edges[i]->data_)->size());
            for( size_t j=0 ; j<weights_diff.size() ; ++j )
            {
                weights_diff[j] *= rcp_batch_size;
            }
            // parallelize only when target size is big enough to mitigate
            // thread spawning overhead.
            bool parallelized = (target.size() >= 512);
            o->update(weights_diff, target, parallelized);
        }
    }
    clearGrads(batch_size);
}

void TdLayer::set_sample_count(size_t sample_count)
{
    for( size_t i=0 ; i<in_channels ; i++ )
    {
        if( !is_trainable_weight(in_type[i]) )
        {
            in_edges[i]->data_.resize(sample_count,
                        in_edges[i]->data_[0]);
        }
        auto def_val = in_edges[i]->grad_[0];
        if( in_edges[i]->grad_.size()!=sample_count )
        {
            in_edges[i]->grad_.resize(sample_count, def_val);
        }
    }

    out_edges->data_.resize(sample_count,
                out_edges->data_[0]);
    auto def_val = out_edges->grad_[0];
    out_edges->grad_.resize(sample_count, def_val);
}

void td_connectLayer(TdLayer *head, TdLayer *tail)
{
    auto out_shape = head->out_shape()[0];
    auto in_shape  = tail->in_shape()[0];

    head->setup(false);

    // todo (karandesai) enable shape inferring for all layers
    // currently only possible for activation layers.
    if( in_shape.size()==0 )
    {
        tail->set_in_shape(out_shape);
        in_shape = out_shape;
    }

    if( !head->out_edges )
    {
        qDebug() << "output edge must not be null";
        exit(0);
    }

    tail->in_edges[0] = head->out_edges;
    tail->in_edges[0]->add_next_node(tail);
}

/// FIXME: check if not used, delete
void data_mismatch(TdLayer *layer, const tiny_dnn::vec_t &data)
{
    std::string lt = layer->layer_type();
    qDebug() << "input dimension mismatch! data dimension:"
             << data.size() << "network dimension:"
             << layer->inDataSize() << "(" << lt.c_str() << ")";//":"
//             << layer->in_shape(). << ")";
}
