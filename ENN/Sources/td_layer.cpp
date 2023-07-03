#include "td_layer.h"

/**
 * @brief Defaul layer constructor that instantiates a N-input, M-output
 *layer
 *
 * @param in_type[N] type of input vector (data, weight, bias...)
 * @param out_type[M] type of output vector
 *
 **/
TdLayer::TdLayer(const std::vector<tiny_dnn::vector_type> &in_type,
                 const std::vector<tiny_dnn::vector_type> &out_type,
                 QObject *parent) : QObject(parent),
                                    node(in_type.size(),
                                         out_type.size())
{
    initialized = false;
    parallelized = true;
    in_channels = in_type.size();
    out_channels = out_type.size();
    this->in_type = in_type;
    this->out_type = out_type;
    weight_init = std::make_shared<tiny_dnn::weight_init::xavier>();
    bias_init   = std::make_shared<tiny_dnn::weight_init::constant>();
    trainable   = true;
}

void TdLayer::setParallelize(bool parallelize)
{
    parallelized = parallelize;
}

void TdLayer::setBackend(std::shared_ptr<tiny_dnn::core::backend> b)
{
    backend = b;
}

void TdLayer::setBackendType(tiny_dnn::core::backend_t bt)
{
    backend_type = bt;
}

bool TdLayer::parallelize()
{
    return parallelized;
}

tiny_dnn::core::backend_t TdLayer::getBackendType()
{
    return backend->type();
}

tiny_dnn::core::backend_t TdLayer::getEngine()
{
    return backend_type;
}

std::string TdLayer::kernelFile()
{
    return std::string("empty_kernel_str");
}

std::string TdLayer::kernelHeader()
{
    return std::string();
}

void TdLayer::createOp()
{

}

void TdLayer::setDevice(const tiny_dnn::Device &device)
{
    device_ptr = const_cast<tiny_dnn::Device *>(&device);
}

tiny_dnn::Device *TdLayer::getDevice()
{
    return device_ptr;
}

std::shared_ptr<tiny_dnn::core::backend> TdLayer::getBackend()
{
    return backend;
}

///< number of incoming edges in this layer
size_t TdLayer::inChannels()
{
    return in_channels;
}

///< number of outgoing edges in this layer
size_t TdLayer::outChannels()
{
    return out_channels;
}

size_t TdLayer::inDataSize() const
{
    return sumif(inShape(),
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
    return sumif(outShape(),
                 [&](size_t i)
    {  // NOLINT
        return out_type[i] == tiny_dnn::vector_type::data;
    }, [](const tiny_dnn::shape3d &s)
    {
        return s.size();
    });
}

std::vector<tiny_dnn::shape3d> TdLayer::inDataShape()
{
    return filter(inShape(), [&](size_t i)
    {  // NOLINT
        return in_type[i] == tiny_dnn::vector_type::data;
    });
}

std::vector<tiny_dnn::shape3d> TdLayer::outDataShape()
{
    return filter(outShape(), [&](size_t i)
    {  // NOLINT
        return out_type[i] == tiny_dnn::vector_type::data;
    });
}

///! @deprecated use in_data_size() instead
size_t TdLayer::inSize() const
{
    return inDataSize();
}

///! @deprecated use out_data_size() instead
size_t TdLayer::outSize() const
{
    return outDataSize();
}

std::vector<const tiny_dnn::vec_t *> TdLayer::weights() const
{
    std::vector<const tiny_dnn::vec_t *> v;
    for( size_t i=0 ; i<in_channels ; i++ )
    {
        if( is_trainable_weight(in_type[i]) )
        {
            v.push_back(getWeightData(i));
        }
    }
    return v;
}

std::vector<tiny_dnn::vec_t *> TdLayer::weights()
{
    std::vector<tiny_dnn::vec_t *> v;
    for( size_t i=0 ; i<in_channels ; i++ )
    {
        if( is_trainable_weight(in_type[i]) )
        {
            v.push_back(getWeightData(i));
        }
    }
    return v;
}

std::vector<tiny_dnn::tensor_t *> TdLayer::weightsGrads()
{
    std::vector<tiny_dnn::tensor_t *> v;
    for( size_t i=0 ; i<in_channels ; i++ )
    {
        if( is_trainable_weight(in_type[i]) )
        {
            v.push_back(ith_in_node(i)->get_gradient());
        }
    }
    return v;
}

std::vector<tiny_dnn::edgeptr_t> TdLayer::inputs()
{
    std::vector<tiny_dnn::edgeptr_t> nodes(in_channels);
    for( size_t i=0 ; i<in_channels ; i++ )
    {
        nodes[i] = ith_in_node(i);
    }
    return nodes;
}

std::vector<tiny_dnn::edgeptr_t> TdLayer::outputs()
{
    std::vector<tiny_dnn::edgeptr_t> nodes(out_channels);
    for( size_t i=0 ; i<out_channels ; i++ )
    {
        nodes[i] = ith_out_node(i);
    }
    return nodes;
}

std::vector<tiny_dnn::edgeptr_t> TdLayer::outputs() const
{
    std::vector<tiny_dnn::edgeptr_t> nodes(out_channels);
    for( size_t i=0 ; i<out_channels ; i++ )
    {
        nodes[i] = const_cast<TdLayer *>(this)->ith_out_node(i);
    }
    return nodes;
}

void TdLayer::setOutGrads(const std::vector<const tiny_dnn::vec_t *> *grad, size_t cnt) {
    CNN_UNREFERENCED_PARAMETER(cnt);
    size_t n = 0;
    for( size_t i=0 ; i<out_channels ; i++ )
    {
        if( out_type[i]!=tiny_dnn::vector_type::data )
        {
            continue;
        }
        tiny_dnn::tensor_t &dst_grad = *ith_out_node(i)->get_gradient();
        assert(n < cnt);
        const auto &src_grad = grad[n++];
        size_t sz            = src_grad.size();
        dst_grad.resize(sz);
        for( size_t j=0 ; j<sz ; ++j )
        {
            assert(dst_grad[j].size() == src_grad[j]->size());
            dst_grad[j] = *src_grad[j];
        }
    }
}

void TdLayer::setInData(const std::vector<const tiny_dnn::vec_t *> *data, size_t cnt)
{
    CNN_UNREFERENCED_PARAMETER(cnt);
    size_t n = 0;
    for( size_t i=0 ; i<in_channels ; i++ )
    {
        if( in_type[i]!=tiny_dnn::vector_type::data )
        {
            continue;
        }
        tiny_dnn::tensor_t &dst_data = *ith_in_node(i)->get_data();
        size_t in_size     = ith_in_node(i)->shape().size();
        assert(n < cnt);
        const auto &src_data = data[n++];
        size_t sz = src_data.size();
        dst_data.resize(sz);

        CNN_UNREFERENCED_PARAMETER(in_size);

        for( size_t j=0 ; j<sz ; ++j )
        {
            assert(src_data[j]->size() == in_size);  // checking if training data is consistent with layer shape
            dst_data[j] = *src_data[j];
        }
    }
}

void TdLayer::setInData(float *data, int len)
{
    if (in_type[0]!=tiny_dnn::vector_type::data)
        return;
    tiny_dnn::tensor_t &dst_data = *ith_in_node(0)->get_data();
    size_t in_size     = ith_in_node(0)->shape().size();
    dst_data.resize(1);

    CNN_UNREFERENCED_PARAMETER(in_size);

    // checking if training data is consistent with layer shape
    assert(len==in_size);
    for( int i=0 ; i<len ; i++ )
    {
        dst_data[0][i] = data[i];
    }
}

void TdLayer::output(std::vector<const tiny_dnn::tensor_t *> &out) const
{
    out.clear();
    for( size_t i=0 ; i<out_channels ; i++ )
    {
        if( out_type[i]==tiny_dnn::vector_type::data )
        {
            out.push_back(ith_out_node(i)->get_data());
        }
    }
}

std::vector<tiny_dnn::vector_type> TdLayer::getInTypes() const
{
    return in_type;
}

std::vector<tiny_dnn::vector_type> TdLayer::getOutTypes() const
{
    return out_type;
}

void TdLayer::setTrainable(bool tr)
{
    trainable = tr;
}

bool TdLayer::getTrainable() const
{
    return trainable;
}

/**
 * return output value range
 * used only for calculating target value from label-id in final(output)
 *layer
 * override properly if the layer is intended to be used as output layer
 **/
std::pair<float_t, float_t> TdLayer::outValueRange() const
{
    return {float_t{0.0}, float_t{1.0}};
}

/**
 * set input shape of a layer (only used internally while shape inferring)
 */
void TdLayer::setInShape(const tiny_dnn::shape3d &in_shape) {
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
size_t TdLayer::fanInSize() const
{
    return inShape()[0].width_;
}

// override to allow initialization of multiple size weight matrices.
size_t TdLayer::fanInSize(size_t) const
{
    return fanInSize();  // fallback to single weight matrix.
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
    return outShape()[0].width_;
}

// override to allow initialization of multiple size weight vectors.
size_t TdLayer::fan_out_size(size_t) const
{
    return fan_out_size();  // fallback to single weight matrix
}

/*by default, we want there to be enough precision*/
void TdLayer::save(std::ostream &os, const int precision) const
{
    os << std::setprecision(precision);
    auto all_weights = weights();
    for( auto &weight:all_weights )
    {
        for( auto w:*weight )
        {
            os << w << " ";
        }
    }
}

/*by default, we want there to be enough precision*/
void TdLayer::load(std::istream &is, const int precision)
{  // NOLINT
    is >> std::setprecision(precision);
    auto all_weights = weights();
    for( auto &weight:all_weights )
    {
        for( auto &w:*weight )
        {
            is >> w;
        }
    }
    initialized = true;
}

void TdLayer::load(const std::vector<float_t> &src, int &idx)
{  // NOLINT
    auto all_weights = weights();
    for( auto &weight:all_weights )
    {
        for( auto &w:*weight )
        {
            w = src[idx++];
        }
    }
    initialized = true;
}

// called afrer updating weight
void TdLayer::postUpdate()
{

}

/**
 * notify changing context (train <=> test)
 **/
void TdLayer::setContext(tiny_dnn::net_phase ctx)
{
    CNN_UNREFERENCED_PARAMETER(ctx);
}

std::vector<tiny_dnn::tensor_t> TdLayer::backward(const std::vector<tiny_dnn::tensor_t> &out_grads)
{  // for test
  setup(false);

  std::vector<std::vector<const tiny_dnn::vec_t *>> grads2;
  grads2.resize(out_grads.size());
  for( size_t i=0 ; i<out_grads.size() ; ++i )
  {
    grads2[i].resize(out_grads[i].size());
    for( size_t j=0 ; j<out_grads[i].size() ; ++j )
    {
      grads2[i][j] = &out_grads[i][j];
    }
  }

  setOutGrads(&grads2[0], grads2.size());
  backward();
  return tiny_dnn::map_<tiny_dnn::tensor_t>(inputs(),
                        [](tiny_dnn::edgeptr_t e) { return *e->get_gradient(); });
}

/* @brief The purpose of this method is to forward the data from the
 * computational graph to the layer interface.
 *
 * This is one of the out of two core (forward/backward) methods that
 * retrieves the data allocated in the heap by the computational graph
 * and constructs the containers to handle the computation by batches.
 * Additionally, the sample count a.k.a number of batches is set.
 *
 * Note: in_data and out_data attempt to contain tensors. However, they
 * are not real tensors since tensor_t have three dimensions instead of
 * four. For this reason they are embedded in to std::vector. Also note
 * that when std::vector<tensor_t*> it's constructed we cannot assure
 * that data is contiguous.
 *
 * After Tensor class integration we should be able to avoid to have
 * in_data and out_data in vectors since Tensor class itself can handle
 * batches storage in one single vector with contiguous data.
 *
 */
void TdLayer::forward()
{
    // the computational graph
    fwd_in_data.resize(in_channels);
    fwd_out_data.resize(out_channels);

    // Organize input/output vectors from storage (computational graph).
    // Internally ith_in_node() will create a connection/edge in the
    // computational graph and will allocate memory in case that it's not
    // done yet.
    for( size_t i=0 ; i<in_channels ; i++ )
    {
        fwd_in_data[i] = ith_in_node(i)->get_data();
    }

    // resize outs and stuff to have room for every input sample in
    // the batch
    setSampleCount(fwd_in_data[0]->size());

    // Internally ith_out_node() will create a connection/edge to the
    // computational graph and will allocate memory in case that it's not
    // done yet. In addition, gradient vector are initialized to default
    // values.
    for( size_t i=0 ; i<out_channels ; i++ )
    {
        fwd_out_data[i] = ith_out_node(i)->get_data();
        ith_out_node(i)->clear_grads();
    }

    // call the forward computation kernel/routine
    forwardPropagation(fwd_in_data, fwd_out_data);
}

void TdLayer::backward()
{
    bwd_in_data.resize(in_channels);
    bwd_in_grad.resize(in_channels);
    bwd_out_data.resize(out_channels);
    bwd_out_grad.resize(out_channels);

    // organize input/output vectors from storage
    for( size_t i = 0; i < in_channels; i++ )
    {
        const auto &nd = ith_in_node(i);
        bwd_in_data[i] = nd->get_data();
        bwd_in_grad[i] = nd->get_gradient();
    }
    for( size_t i = 0; i < out_channels; i++ )
    {
        const auto &nd  = ith_out_node(i);
        bwd_out_data[i] = nd->get_data();
        bwd_out_grad[i] = nd->get_gradient();
    }
    backPropagation(bwd_in_data, bwd_out_data, bwd_out_grad, bwd_in_grad);
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
    if( inShape().size()!=in_channels ||
            outShape().size()!=out_channels )
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
    for( size_t i=0 ; i<out_channels ; i++ )
    {
        if( !next_[i] )
        {
            // connection edge doesn't exist, so we proceed to allocate the
            // necessary memory.
            next_[i] = std::make_shared<tiny_dnn::edge>(this, outShape()[i],
                                                        out_type[i]);
        }
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
        switch( in_type[i] )
        {
        // fill vectors of weight type
        case tiny_dnn::vector_type::weight:
            weight_init->fill(getWeightData(i), fanInSize(i),
                              fan_out_size(i));
            break;
            // fill vector of bias type
        case tiny_dnn::vector_type::bias:
            bias_init->fill(getWeightData(i), fanInSize(i), fan_out_size(i));
            break;
        default: break;
        }
    }
    // in case we succeed with data initialization, we mark the
    // layer/node as initialized.
    initialized = true;
}

void TdLayer::clearGrads()
{
    for( size_t i=0 ; i<in_type.size() ; i++ )
    {
        ith_in_node(i)->clear_grads();
    }
}

void TdLayer::updateWeight(tiny_dnn::optimizer *o)
{
    auto &diff = weights_diff;
    for( size_t i=0; i<in_type.size() ; i++ )
    {
        if( getTrainable() && is_trainable_weight(in_type[i]) )
        {
            tiny_dnn::vec_t &target = *getWeightData(i);
            ith_in_node(i)->merge_grads(&diff);
            float_t rcp_batch_size =
                    float_t(1.0) / float_t(ith_in_node(i)->get_data()->size());
            for( size_t j=0 ; j<diff.size() ; ++j )
            {
                diff[j] *= rcp_batch_size;
            }
            // parallelize only when target size is big enough to mitigate
            // thread spawning overhead.
            bool parallelized = (target.size() >= 512);
            o->update(diff, target, parallelized);
        }
    }
    clearGrads();
    postUpdate();
}

bool TdLayer::hasSameWeights(const TdLayer &rhs, float_t eps) const
{
    auto w1 = weights();
    auto w2 = rhs.weights();
    if ( w1.size()!=w2.size() )
    {
        return false;
    }

    for( size_t i=0 ; i<w1.size() ; i++ )
    {
        if( w1[i]->size()!=w2[i]->size() )
        {
            return false;
        }

        for( size_t j=0 ; j<w1[i]->size() ; j++ )
        {
            if( std::abs(w1[i]->at(j)-w2[i]->at(j))>eps )
            {
                return false;
            }
        }
    }
    return true;
}

void TdLayer::setSampleCount(size_t sample_count)
{
    // increase the size if necessary - but do not decrease
    auto resize = [sample_count](tiny_dnn::tensor_t *tensor)
    {
        tensor->resize(sample_count, (*tensor)[0]);
    };

    for( size_t i=0 ; i<in_channels ; i++ )
    {
        if( !is_trainable_weight(in_type[i]) )
        {
            resize(ith_in_node(i)->get_data());
        }
        resize(ith_in_node(i)->get_gradient());
    }

    for( size_t i=0 ; i<out_channels ; i++ )
    {
        if( !is_trainable_weight(out_type[i]) )
        {
            resize(ith_out_node(i)->get_data());
        }
        resize(ith_out_node(i)->get_gradient());
    }
}

template<typename WeightInit>
TdLayer &TdLayer::weightInit(const WeightInit &f)
{
    weight_init = std::make_shared<WeightInit>(f);
    return *this;
}

template<typename BiasInit>
TdLayer &TdLayer::biasInit(const BiasInit &f)
{
    bias_init = std::make_shared<BiasInit>(f);
    return *this;
}

template<typename WeightInit>
TdLayer &TdLayer::weightInit(std::shared_ptr<WeightInit> f)
{
    weight_init = f;
    return *this;
}

template<typename BiasInit>
TdLayer &TdLayer::biasInit(std::shared_ptr<BiasInit> f)
{
    bias_init = f;
    return *this;
}
