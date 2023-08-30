#include "td_network.h"

TdNetwork::TdNetwork(QString name, QObject *parent) :
    QObject(parent)
{
    net_name = name;
    stop_training = 0;
}

void TdNetwork::initWeightBias()
{
     setup(true);
}

tiny_dnn::vec_t TdNetwork::predict(const tiny_dnn::vec_t &in)
{
    if( in.size()!=(size_t)inDataSize() )
    {
        data_mismatch(nod[0], in);
    }
    // a workaround to reduce memory consumption by skipping wrapper
    // function
    std::vector<tiny_dnn::tensor_t> a(1);
    a[0].emplace_back(in);
    return forward(a)[0][0];
}

void TdNetwork::updateWeights(tiny_dnn::optimizer *opt)
{
    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->updateWeight(opt);
    }
}

float_t TdNetwork::predictMaxValue(const tiny_dnn::vec_t &in)
{
    return fprop_max(in);
}

/**
* executes forward-propagation and returns maximum output index
**/
tiny_dnn::label_t TdNetwork::predictLabel(
        const tiny_dnn::vec_t &in)
{
    return fprop_max_index(in);
}

/**
* set the netphase to train or test
* @param phase phase of network, could be train or test
*/
void TdNetwork::setNetPhase(tiny_dnn::net_phase phase)
{
    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->set_context(phase);
    }
}

/**
* request to finish an ongoing training
*
* It is safe to test the current network performance in @a
* on_batch_enumerate
* and
* @a on_epoch_enumerate callbacks during training.
*/
void TdNetwork::stopOngoingTraining()
{
    stop_training = true;
}

std::vector<tiny_dnn::vec_t> TdNetwork::test(
        const std::vector<tiny_dnn::vec_t> &in)
{
    std::vector<tiny_dnn::vec_t> test_result(in.size());
    setNetPhase(tiny_dnn::net_phase::test);
    for( size_t i=0 ; i<in.size() ; i++ )
    {
        test_result[i] = predict(in[i]);
    }
    return test_result;
}

float_t TdNetwork::getLoss(const std::vector<tiny_dnn::vec_t> &in,
                     const std::vector<tiny_dnn::label_t> &t)
{
    float_t sum_loss = float_t(0);

    std::vector<tiny_dnn::tensor_t> label_tensor;
    normalizeTensor(t, label_tensor);

    for( size_t i=0 ; i<in.size() ; i++ )
    {
        const tiny_dnn::vec_t predicted = predict(in[i]);
        for( size_t j=0 ; j<1 ; j++ )
        {
            sum_loss += tiny_dnn::mse::f(predicted,
                                    label_tensor[i][j]);
        }
    }
    return sum_loss;
}

float_t TdNetwork::getLoss(const std::vector<tiny_dnn::vec_t> &in,
                     const std::vector<tiny_dnn::vec_t> &t)
{
    float_t sum_loss = float_t(0);

    for( size_t i=0 ; i<in.size() ; i++ )
    {
        const tiny_dnn::vec_t predicted = predict(in[i]);
        sum_loss += tiny_dnn::mse::f(predicted, t[i]);
    }
    return sum_loss;
}

size_t TdNetwork::layerSize()
{
    return nod.size();
}

/**
* @deprecated use layer_size() instread.
**/
size_t TdNetwork::depth()
{
    return layerSize();
}

float_t TdNetwork::fprop_max(const tiny_dnn::vec_t &in)
{
    const tiny_dnn::vec_t &prediction = predict(in);
    return *std::max_element(std::begin(prediction),
                             std::end(prediction));
}

tiny_dnn::label_t TdNetwork::fprop_max_index(
        const tiny_dnn::vec_t &in)
{
    return tiny_dnn::label_t(max_index(predict(in)));
}

/**
* train on one minibatch
*
* @param size is the number of data points to use in this batch
*/
void TdNetwork::trainMiniBatch(tiny_dnn::adagrad &optimizer,
                           const tiny_dnn::tensor_t *in,
                           const tiny_dnn::tensor_t *t,
                           int size, const tiny_dnn::tensor_t *t_cost)
{
    std::vector<tiny_dnn::tensor_t> in_batch;
    std::vector<tiny_dnn::tensor_t> t_batch;
    in_batch.resize(size);
    t_batch.resize(size);

    std::copy(&in[0], &in[0] + size, &in_batch[0]);
    std::copy(&t[0], &t[0] + size, &t_batch[0]);
    std::vector<tiny_dnn::tensor_t> t_cost_batch;
    t_cost_batch = std::vector<tiny_dnn::tensor_t>(&t_cost[0],
        &t_cost[0] + size);

    std::vector<tiny_dnn::tensor_t> o_batch;
    o_batch = forward(in_batch);

    // back propagation
    std::vector<tiny_dnn::tensor_t> delta;
    delta = tiny_dnn::gradient<tiny_dnn::mse>(o_batch, t_batch,
                                              t_cost_batch);
    backward(delta);
    updateWeights(&optimizer);
}

void TdNetwork::checkTargetCostMatrix(
        const std::vector<tiny_dnn::tensor_t> &t,
        const std::vector<tiny_dnn::tensor_t> &t_cost)
{
    if( !t_cost.empty() )
    {
        if( t.size()!=t_cost.size() )
        {
            qDebug() << "if target cost is supplied, "
                        "its length must equal that of target data";
            exit(0);
        }

        size_t end = t.size();
        for( size_t i=0 ; i<end ; i++ )
        {
            checkTargetCostElement(t[i], t_cost[i]);
        }
    }
}

// regression
void TdNetwork::checkTargetCostElement(const tiny_dnn::vec_t &t,
                                       const tiny_dnn::vec_t &t_cost)
{
    if( t.size()!=t_cost.size() )
    {
        qDebug() << "if target cost is supplied for a regression task, "
                    "its shape must be identical to the target data";
        exit(0);
    }
}

void TdNetwork::checkTargetCostElement(const tiny_dnn::tensor_t &t,
                                       const tiny_dnn::tensor_t &t_cost)
{
    if( t.size()!=t_cost.size() )
    {
        qDebug() << "if target cost is supplied for a regression task, "
                    "its shape must be identical to the target data";
        exit(0);
    }
    for( size_t i=0 ; i<t.size() ; i++ )
    {
        checkTargetCostElement(t[i], t_cost[i]);
    }
}

template<typename T>
float_t TdNetwork::getLoss(const std::vector<T> &in,
                           const std::vector<tiny_dnn::tensor_t> &t)
{
    float_t sum_loss = float_t(0);
    std::vector<tiny_dnn::tensor_t> in_tensor;
    normalizeTensor(in, in_tensor);

    for( size_t i=0 ; i<in.size() ; i++ )
    {
        tiny_dnn::tensor_t predicted;
        std::vector<tiny_dnn::tensor_t> ten = {in_tensor[i]};
        predicted = forward(ten)[0];
        for( size_t j=0 ; j<predicted.size() ; j++ )
        {
            sum_loss += tiny_dnn::mse::f(predicted[j],
                                                   t[i][j]);
        }
    }
    return sum_loss;
}

bool TdNetwork::fit(tiny_dnn::adagrad &optimizer,
                const std::vector<tiny_dnn::tensor_t> &inputs,
                const std::vector<tiny_dnn::tensor_t> &desired_outputs,
                size_t batch_size, int epoch,
                const bool reset_weights,
                const std::vector<tiny_dnn::tensor_t> &t_cost)
{
    checkTargetCostMatrix(desired_outputs, t_cost);
    setNetPhase(tiny_dnn::net_phase::train);
    setup(reset_weights);

    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->setParallelize(true);
    }
    optimizer.reset();
    stop_training = false;
    int len_input = inputs.size();
    for( int iter=0 ; iter<epoch && !stop_training ; iter++ )
    {
        for( int i=0 ; i<len_input && !stop_training ;
             i+=batch_size )
        {
            int min_size = std::min(batch_size, inputs.size() - i);
            trainMiniBatch(optimizer, &inputs[i], &desired_outputs[i],
                       min_size, &(t_cost[i]));

            emit onBatchEnumerate();
        }
        emit OnEpochEnumerate();
    }
    setNetPhase(tiny_dnn::net_phase::test);
    return true;
}

void TdNetwork::normalizeTensor(
        const std::vector<tiny_dnn::tensor_t> &inputs,
        std::vector<tiny_dnn::tensor_t> &normalized)
{
    normalized = inputs;
}

void TdNetwork::normalizeTensor(
        const std::vector<tiny_dnn::vec_t> &inputs,
        std::vector<tiny_dnn::tensor_t> &normalized)
{
    normalized.reserve(inputs.size());
    for( size_t i=0 ; i<inputs.size() ; i++ )
    {
        normalized.emplace_back(tiny_dnn::tensor_t{inputs[i]});
    }
}

void TdNetwork::normalizeTensor(
        const std::vector<tiny_dnn::label_t> &inputs,
        std::vector<tiny_dnn::tensor_t> &normalized)
{
    std::vector<tiny_dnn::vec_t> vec;
    normalized.reserve(inputs.size());
    label2vec(&inputs[0], inputs.size(), vec);
    normalizeTensor(vec, normalized);
}

void TdNetwork::label2vec(const tiny_dnn::label_t *t, size_t num,
                             std::vector<tiny_dnn::vec_t> &vec)
{
    size_t outdim = outDataSize();
    int target_value_min = 0;
    int target_value_max = 1;

    vec.reserve(num);
    for( size_t i=0 ; i<num ; i++ )
    {
        assert(t[i] < outdim);
        tiny_dnn::vec_t buf(outdim, target_value_min);
        buf[t[i]] = target_value_max;
        vec.push_back(buf);
    }
}

TdNetwork* TdNetwork::addFC(int in_dim, int out_dim)
{
    TdFC *fc = new TdFC(in_dim, out_dim);
    nod.push_back(fc);
    td_connectHeadToTail(&nod);
    return this;
}

TdNetwork* TdNetwork::addLeakyRelu()
{
    TdLeakyRelu *lr = new TdLeakyRelu();
    nod.push_back(lr);
    td_connectHeadToTail(&nod);
    return this;
}

TdNetwork* TdNetwork::addConv(int in_width, int in_height,
                        int window_width, int window_height,
                        int in_channels, int out_channels)
{
    TdConvolution *conv = new TdConvolution(in_width, in_height,
                      window_width, window_height, in_channels,
                      out_channels);
    nod.push_back(conv);
    td_connectHeadToTail(&nod);
    return this;
}

TdNetwork* TdNetwork::addAvePool(int in_width, int in_height, int in_channels,
                int pool_size_x, int pool_size_y, int stride_x,
                int stride_y)
{
    TdAvePool *ap = new TdAvePool(in_width, in_height,
                in_channels, pool_size_x, pool_size_y, stride_x,
                stride_y);
    nod.push_back(ap);
    td_connectHeadToTail(&nod);
    return this;
}

TdNetwork* TdNetwork::addSoftMax()
{
    TdSoftmax *sm = new TdSoftmax;
    nod.push_back(sm);
    td_connectHeadToTail(&nod);
    return this;
}

void TdNetwork::backward(
        const std::vector<tiny_dnn::tensor_t> &first)
{
    std::vector<std::vector<const tiny_dnn::vec_t *>> reordered_grad;
    td_reorder(first, reordered_grad);
    assert(reordered_grad.size() == 1);

    nod.back()->setOutGrads(&reordered_grad[0], 1);

    int len = nod.size();
    for( int i=len ; i>0 ; i-- )
    {
        nod[i-1]->backward();
    }
}

std::vector<tiny_dnn::tensor_t> TdNetwork::forward(
        const std::vector<tiny_dnn::tensor_t> &first)
{
    nod.front()->setInData(first);

    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->forward();
    }

    std::vector<const tiny_dnn::tensor_t *> out;
    nod.back()->output(out);

    return td_normalizeOut(out);
}

tiny_dnn::vec_t TdNetwork::forward(tiny_dnn::vec_t &first)
{
    nod.front()->setInData(first);

    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->forward();
    }

    std::vector<const tiny_dnn::tensor_t *> out;
    nod.back()->output(out);

    return td_normalizeOut(out);
}

/**
 * setup all weights, must be called before forward/backward
 **/
void TdNetwork::setup(bool reset_weight)
{
    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->setup(reset_weight);
    }
}

void TdNetwork::clearGrads()
{
    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->clearGrads();
    }
}

size_t TdNetwork::inDataSize()
{
    return nod.front()->inDataSize();
}

size_t TdNetwork::outDataSize()
{
    return nod.back()->outDataSize();
}

