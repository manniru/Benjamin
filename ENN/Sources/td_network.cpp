#include "td_network.h"

TdNetwork::TdNetwork(int bs, QObject *parent) :
    QObject(parent)
{
    worker = new TdWorker(&nod);
    worker->stop_training = false;
    setBatchSize(bs);

    connect(worker, SIGNAL(onEpochEnumerate()),
            this, SLOT(epochEnumerate()));
    connect(worker, SIGNAL(epochFinished()),
            this, SLOT(workerFinished()));
    worker_finished = false;
}

void TdNetwork::initWeightBias()
{
     setup(true);
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
    worker->stop_training = true;
}

void TdNetwork::epochEnumerate()
{
    emit onEpochEnumerate();
}

void TdNetwork::workerFinished()
{
    worker_finished = true;
}

void TdNetwork::fit(tiny_dnn::tensor_t &inputs,
                std::vector<tiny_dnn::tensor_t> &desired_outputs,
                int epoch, bool reset_weights,
                tiny_dnn::tensor_t &t_cost)
{
    setup(reset_weights);

    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->setParallelize(true);
    }
    worker_finished = false;
    worker->fit(&inputs, &desired_outputs, epoch, &t_cost);
    while( !worker_finished )
    {
        QThread::msleep(10);
        QGuiApplication::processEvents();
    }
}

void TdNetwork::setBatchSize(int bs)
{
    worker->setBatchSize(bs);
}

void TdNetwork::normalizeTensor(
        const std::vector<tiny_dnn::label_t> &inputs,
        std::vector<tiny_dnn::tensor_t> &normalized)
{
    std::vector<tiny_dnn::vec_t> vec;
    normalized.reserve(inputs.size());
    label2vec(&inputs[0], inputs.size(), vec);
    int len = vec.size();
    normalized.reserve(len);
    for( int i=0 ; i<len ; i++ )
    {
        normalized.emplace_back(tiny_dnn::tensor_t{vec[i]});
    }
}

void TdNetwork::label2vec(const tiny_dnn::label_t *t, size_t num,
                             std::vector<tiny_dnn::vec_t> &vec)
{
    size_t outdim = nod.back()->outDataSize();
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

tiny_dnn::vec_t TdNetwork::predict(tiny_dnn::vec_t &first)
{
    nod.front()->setInData(first);

    TdMiniWorker test_worker(&nod, NULL, NULL);
    test_worker.setRange(0, 1);

    return test_worker.forward()[0][0];
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
