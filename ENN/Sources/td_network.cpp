#include "td_network.h"

TdNetwork::TdNetwork(QString name, QObject *parent) : QObject(parent)
{
    net_name = name;
    stop_training = 0;
}

void TdNetwork::initWeight()
{
     net.setup(true);
}

// convenience wrapper for the function below
void TdNetwork::bprop(const std::vector<tiny_dnn::vec_t> &out,
                      const std::vector<tiny_dnn::vec_t> &t,
                      const std::vector<tiny_dnn::vec_t> &t_cost)
{
    bprop(std::vector<tiny_dnn::tensor_t>{out},
          std::vector<tiny_dnn::tensor_t>{t},
          std::vector<tiny_dnn::tensor_t>{t_cost});
}

void TdNetwork::bprop(const std::vector<tiny_dnn::tensor_t> &out,
                      const std::vector<tiny_dnn::tensor_t> &t,
                      const std::vector<tiny_dnn::tensor_t> &t_cost)
{
    std::vector<tiny_dnn::tensor_t> delta =
            tiny_dnn::gradient<tiny_dnn::cross_entropy>(out, t, t_cost);
    net.backward(delta);
}

// convenience wrapper for the function below
std::vector<tiny_dnn::vec_t> TdNetwork::predict(const std::vector<tiny_dnn::vec_t> &in)
{
    return predict(std::vector<tiny_dnn::tensor_t>{in})[0];
}

std::vector<tiny_dnn::tensor_t> TdNetwork::predict(const std::vector<tiny_dnn::tensor_t> &in)
{
    return net.forward(in);
}

tiny_dnn::vec_t TdNetwork::predict(const tiny_dnn::vec_t &in)
{
    if( in.size()!=(size_t)inDataSize() )
    {
        data_mismatch(**net.begin(), in);
    }
    // a workaround to reduce memory consumption by skipping wrapper
    // function
    std::vector<tiny_dnn::tensor_t> a(1);
    a[0].emplace_back(in);
    return predict(a)[0][0];
}

void TdNetwork::updateWeights(tiny_dnn::optimizer *opt)
{
    for(auto l : net)
    {
        l->update_weight(opt);
    }
}

float_t TdNetwork::predictMaxValue(const tiny_dnn::vec_t &in)
{
    return fprop_max(in);
}

/**
* executes forward-propagation and returns maximum output index
**/
tiny_dnn::label_t TdNetwork::predictLabel(const tiny_dnn::vec_t &in)
{
    return fprop_max_index(in);
}

/**
* set the netphase to train or test
* @param phase phase of network, could be train or test
*/
void TdNetwork::setNetPhase(tiny_dnn::net_phase phase)
{
    for (auto n : net)
    {
        n->set_context(phase);
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
    for (size_t i = 0; i < in.size(); i++)
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

    for (size_t i = 0; i < in.size(); i++) {
        const tiny_dnn::vec_t predicted = predict(in[i]);
        for (size_t j = 0; j < 1; j++) {
            sum_loss += tiny_dnn::cross_entropy::f(predicted,
                                                   label_tensor[i][j]);
        }
    }
    return sum_loss;
}

float_t TdNetwork::getLoss(const std::vector<tiny_dnn::vec_t> &in,
                           const std::vector<tiny_dnn::vec_t> &t) {
    float_t sum_loss = float_t(0);

    for (size_t i = 0; i < in.size(); i++) {
        const tiny_dnn::vec_t predicted = predict(in[i]);
        sum_loss += tiny_dnn::cross_entropy::f(predicted, t[i]);
    }
    return sum_loss;
}

size_t TdNetwork::layerSize()
{
    return net.size();
}

/**
* @deprecated use layer_size() instread.
**/
size_t TdNetwork::depth()
{
    return layerSize();
}

tiny_dnn::layer* TdNetwork::operator[](size_t index)
{
    return net[index];
}

size_t TdNetwork::outDataSize()
{
    return net.outDataSize();
}

size_t TdNetwork::inDataSize()
{
    return net.inDataSize();
}

void TdNetwork::load(const std::string &filename)
{
    std::ifstream ifs(filename.c_str(), std::ios::binary | std::ios::in);
    if( ifs.fail() || ifs.bad() )
    {
        qDebug() << "failed to open:" << &filename;
    }
    cereal::BinaryInputArchive bi(ifs);
    fromArchive(bi);
}

void TdNetwork::save(const std::string &filename)
{
    std::ofstream ofs(filename.c_str(), std::ios::binary |
                      std::ios::out);
    if( ofs.fail() || ofs.bad() )
    {
        qDebug() << "failed to open:" << &filename;
        exit(0);
    }
    cereal::BinaryOutputArchive bo(ofs);
    toArchive(bo);
}

void TdNetwork::toArchive(cereal::BinaryOutputArchive &ar)
{
    net.saveModel<cereal::BinaryOutputArchive>(ar);
    net.saveWeights<cereal::BinaryOutputArchive>(ar);
}

void TdNetwork::fromArchive(cereal::BinaryInputArchive &ar)
{
    net.loadModel<cereal::BinaryInputArchive>(ar);
    net.loadWeights<cereal::BinaryInputArchive>(ar);
}

float_t TdNetwork::fprop_max(const tiny_dnn::vec_t &in)
{
    const tiny_dnn::vec_t &prediction = predict(in);
    return *std::max_element(std::begin(prediction),
                             std::end(prediction));
}

tiny_dnn::label_t TdNetwork::fprop_max_index(const tiny_dnn::vec_t &in)
{
    return tiny_dnn::label_t(max_index(predict(in)));
}

/**
* train on one minibatch
*
* @param size is the number of data points to use in this batch
*/
void TdNetwork::train_once(tiny_dnn::adagrad &optimizer, const tiny_dnn::tensor_t *in, const tiny_dnn::tensor_t *t, int size, const int nbThreads, const tiny_dnn::tensor_t *t_cost)
{
    if(size == 1)
    {
        bprop(predict(in[0]), t[0],
                t_cost ? t_cost[0] : tiny_dnn::tensor_t());
        net.updateWeights(&optimizer);
    }
    else
    {
        train_onebatch(optimizer, in, t, size, nbThreads, t_cost);
    }
}

/**
* trains on one minibatch, i.e. runs forward and backward propagation to
* calculate
* the gradient of the loss function with respect to the network parameters
* (weights),
* then calls the optimizer algorithm to update the weights
*
* @param batch_size the number of data points to use in this batch
*/
void TdNetwork::train_onebatch(tiny_dnn::adagrad &optimizer,
                               const tiny_dnn::tensor_t *in,
                               const tiny_dnn::tensor_t *t,
                               int batch_size, const int num_tasks,
                               const tiny_dnn::tensor_t *t_cost)
{
    CNN_UNREFERENCED_PARAMETER(num_tasks);
    std::copy(&in[0], &in[0] + batch_size, &in_batch[0]);
    std::copy(&t[0], &t[0] + batch_size, &t_batch[0]);
    std::vector<tiny_dnn::tensor_t> t_cost_batch =
            t_cost ? std::vector<tiny_dnn::tensor_t>(&t_cost[0],
            &t_cost[0] + batch_size)
            : std::vector<tiny_dnn::tensor_t>();

    bprop(predict(in_batch), t_batch, t_cost_batch);
    net.updateWeights(&optimizer);
}

bool TdNetwork::calcDelta(const std::vector<tiny_dnn::tensor_t> &in,
                           const std::vector<tiny_dnn::tensor_t> &v,
                           tiny_dnn::vec_t &w, tiny_dnn::tensor_t &dw,
                           size_t check_index, double eps)
{
    static const float_t delta =
            std::sqrt(std::numeric_limits<float_t>::epsilon());

    assert(in.size() == v.size());

    const size_t sample_count = in.size();

    assert(sample_count > 0);

    // at the moment, channel count must be 1
    assert(in[0].size() == 1);
    assert(v[0].size() == 1);

    // clear previous results, if any
    for( tiny_dnn::vec_t &dw_sample:dw )
    {
        vectorize::fill(&dw_sample[0], dw_sample.size(), float_t(0));
    }

    // calculate dw/dE by numeric
    float_t prev_w = w[check_index];

    float_t f_p    = float_t(0);
    w[check_index] = prev_w + delta;
    for (size_t i = 0; i < sample_count; i++) {
        f_p += getLoss(in[i], v[i]);
    }

    float_t f_m    = float_t(0);
    w[check_index] = prev_w - delta;
    for( size_t i=0; i<sample_count ; i++ )
    {
        f_m += getLoss(in[i], v[i]);
    }

    float_t delta_by_numerical = (f_p - f_m) / (float_t(2) * delta);
    w[check_index] = prev_w;

    // calculate dw/dE by bprop
    bprop(predict(in), v, std::vector<tiny_dnn::tensor_t>());

    float_t delta_by_bprop = 0;
    for( size_t sample=0 ; sample<sample_count ; ++sample )
    {
        delta_by_bprop += dw[sample][check_index];
    }
    net.clearGrads();

    return std::abs(delta_by_bprop - delta_by_numerical) <= eps;
}

void TdNetwork::checkTargetCostMatrix(const std::vector<tiny_dnn::tensor_t> &t,
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

        for( size_t i=0, end=t.size() ; i<end; i++ )
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

const tiny_dnn::tensor_t *TdNetwork::getTargetCostSamplePointer(
        const std::vector<tiny_dnn::tensor_t> &t_cost, size_t i)
{
    if( !t_cost.empty() )
    {
        assert(i < t_cost.size());
        return &(t_cost[i]);
    }
    else
    {
        return nullptr;
    }
}

/**
* trains the network for a fixed number of epochs to generate desired
* output.
*
* This method executes fixed number of training steps and invoke callbacks
* for
* each mini-batch/epochs.
* The network is trained to minimize given loss function(specified by
* template
* parameter).
*
* Shape of inputs and desired_outputs must be same to network inputs. For
* example, if your network
* has 2 input layers that takes N dimensional array, for each element of
* inputs must be [2xN]
* array.
*
* @code
* network<sequential> net;
* adagrad opt;
*
* net << layers::fc(2, 3) << activation::tanh()
*     << layers::fc(3, 1) << activation::relu();
*
* // 2training data, each data is float_t[2]
* std::vector<vec_t> data { { 1, 0 }, { 0, 2 } };
* std::vector<vec_t> out  {    { 2 },    { 1 } };
*
* net.fit<mse>(opt, data, out, 1, 1);
*
* // 2training data, each data is float_t[1][2]
* // this form is also valid
* std::vector<tensor_t> data2{ { { 1, 0 } }, { { 0, 2 } } };
* std::vector<tensor_t> out2 { {    { 2 } }, {    { 1 } } };
*
* net.fit<mse>(opt, data2, out2, 1, 1);
* @endcode
*
*
* @param optimizer          optimizing algorithm for training
* @param inputs             array of input data
* @param desired_outputs    array of desired output
* @param batch_size         number of samples per parameter update
* @param epoch              number of training epochs
* @param on_batch_enumerate callback for each mini-batch enumerate
* @param on_epoch_enumerate callback for each epoch
* @param reset_weights      set true if reset current network weights
* @param n_threads          number of tasks
* @param t_cost             target costs (leave to nullptr in order to
* assume
* equal cost for every target)
*/
template<typename T, typename U>
bool TdNetwork::fit(tiny_dnn::adagrad &optimizer,
                    const std::vector<T> &inputs,
                    const std::vector<U> &desired_outputs,
                    size_t batch_size, int epoch,
                    const bool reset_weights, const int n_threads,
                    const std::vector<U> &t_cost)
{
    std::vector<tiny_dnn::tensor_t> input_tensor, output_tensor,
            t_cost_tensor;
    normalizeTensor(inputs, input_tensor);
    normalizeTensor(desired_outputs, output_tensor);
    if( !t_cost.empty() )
    {
        normalizeTensor(t_cost, t_cost_tensor);
    }

    return fit(optimizer, input_tensor, output_tensor, batch_size, epoch,
               reset_weights, n_threads, t_cost_tensor);
}

/**
* @param optimizer          optimizing algorithm for training
* @param inputs             array of input data
* @param desired_outputs    array of desired output
* @param batch_size         number of samples per parameter update
* @param epoch              number of training epochs
**/
template<typename T, typename U>
bool TdNetwork::fit(tiny_dnn::adagrad &optimizer,
                    const std::vector<T> &inputs,
                    const std::vector<U> &desired_outputs,
                    size_t batch_size, int epoch)
{
    return fit<tiny_dnn::cross_entropy>(optimizer, inputs,
                                        desired_outputs, batch_size,
                                        epoch, tiny_dnn::nop,
                                        tiny_dnn::nop);
}

template<typename T>
float_t TdNetwork::getLoss(const std::vector<T> &in,
                           const std::vector<tiny_dnn::tensor_t> &t)
{
    float_t sum_loss = float_t(0);
    std::vector<tiny_dnn::tensor_t> in_tensor;
    normalizeTensor(in, in_tensor);

    for (size_t i = 0; i < in.size(); i++)
    {
        const tiny_dnn::tensor_t predicted = predict(in_tensor[i]);
        for (size_t j = 0; j < predicted.size(); j++)
        {
            sum_loss += tiny_dnn::cross_entropy::f(predicted[j],
                                                   t[i][j]);
        }
    }
    return sum_loss;
}

/**
* return index-th layer as <T>
* throw nn_error if index-th layer cannot be converted to T
**/
template<typename T> T &TdNetwork::at(size_t index)
{
    return net.template at<T>(index);
}

template<typename WeightInit>
TdNetwork &TdNetwork::weightInit(const WeightInit &f)
{
    auto ptr = std::make_shared<WeightInit>(f);
    for( auto &l:net )
    {
        l->weight_init(ptr);
    }
    return *this;
}

template<typename BiasInit>
TdNetwork &TdNetwork::biasInit(const BiasInit &f)
{
    auto ptr = std::make_shared<BiasInit>(f);
    for( auto &l:net )
    {
        l->bias_init(ptr);
    }
    return *this;
}

bool TdNetwork::fit(tiny_dnn::adagrad &optimizer,
                    const std::vector<tiny_dnn::tensor_t> &inputs,
                    const std::vector<tiny_dnn::tensor_t> &desired_outputs,
                    size_t batch_size, int epoch,
                    const bool reset_weights, const int n_threads,
                    const std::vector<tiny_dnn::tensor_t> &t_cost)
{
    checkTargetCostMatrix(desired_outputs, t_cost);
    setNetPhase(tiny_dnn::net_phase::train);
    net.setup(reset_weights);

    for( auto n:net )
    {
        n->set_parallelize(true);
    }
    optimizer.reset();
    stop_training = false;
    in_batch.resize(batch_size);
    t_batch.resize(batch_size);
    for( int iter=0 ; iter<epoch && !stop_training ; iter++ )
    {
        for( size_t i=0 ; i<inputs.size() && !stop_training ;
             i+=batch_size )
        {
            int min_size = std::min(batch_size, inputs.size() - i);
            const tiny_dnn::tensor_t *cost =
                    getTargetCostSamplePointer(t_cost, i);
            train_once(optimizer, &inputs[i], &desired_outputs[i],
                       min_size, n_threads, cost);

            emit onBatchEnumerate();
        }
        emit OnEpochEnumerate();
    }
    setNetPhase(tiny_dnn::net_phase::test);
    return true;
}

void TdNetwork::normalizeTensor(const std::vector<tiny_dnn::tensor_t> &inputs,
                                std::vector<tiny_dnn::tensor_t> &normalized)
{
    normalized = inputs;
}

void TdNetwork::normalizeTensor(const std::vector<tiny_dnn::vec_t> &inputs,
                                std::vector<tiny_dnn::tensor_t> &normalized)
{
    normalized.reserve(inputs.size());
    for(size_t i = 0; i < inputs.size(); i++)
    {
        normalized.emplace_back(tiny_dnn::tensor_t{inputs[i]});
    }
}

void TdNetwork::normalizeTensor(const std::vector<tiny_dnn::label_t> &inputs,
                                std::vector<tiny_dnn::tensor_t> &normalized)
{
    std::vector<tiny_dnn::vec_t> vec;
    normalized.reserve(inputs.size());
    net.label2vec(inputs, vec);
    normalizeTensor(vec, normalized);
}
