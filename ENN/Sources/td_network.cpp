#include "td_network.h"
#include <QGuiApplication>

TdNetwork::TdNetwork(int bs, QObject *parent) :
    QObject(parent)
{
    setBatchSize(bs);
    stop_training = 0;
}

void TdNetwork::initWeightBias()
{
     setup(true);
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

/**
* train on one minibatch
*
* @param size is the number of data points to use in this batch
*/
void TdNetwork::trainMiniBatch(std::vector<tiny_dnn::tensor_t> &in,
                           tiny_dnn::tensor_t *t,
                           int data_size,
                           tiny_dnn::tensor_t *t_cost,
                           int offset)
{
    std::copy(&t[0], &t[0] + data_size, &t_batch[0]);
    t_cost_batch = std::vector<tiny_dnn::tensor_t>(&t_cost[0],
        &t_cost[0] + data_size);
    nod.front()->setInData(in, data_size, offset);

    int nod_len = nod.size();
    for( int i=0 ; i<nod_len ; i++ )
    {
        // resize outs and stuff to have room for every input sample in
        // the batch
        nod[i]->set_sample_count(data_size);
        int ch_len = nod[i]->out_channels;
        for(int j=0 ; j<ch_len ; j++ )
        {
            tiny_dnn::tensor_t &dst_grad = nod[i]->ith_out_node(j)->grad_;
            dst_grad.resize(data_size);
        }
    }

    int thread_num = TD_THREAD_NUM;
    int data_per_thread = data_size/thread_num;
    if( data_per_thread==0 )
    {
        data_per_thread = 1;
    }

    worker_len = data_size;
    worker_done = 0;

    // should call resize for all layers before !for!
    for( int i=0 ; i<workers.length() ; i++ )
    {
        if( i<data_size )
        {
            workers[i]->setRange(i*data_per_thread,
                                 (i+1)*data_per_thread);
            workers[i]->enabled = true;
        }
        else
        {
            workers[i]->enabled = false;
        }
    }
    emit startWorkers();
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

void TdNetwork::fit(std::vector<tiny_dnn::tensor_t> &inputs,
                std::vector<tiny_dnn::tensor_t> &desired_outputs,
                int epoch, bool reset_weights,
                std::vector<tiny_dnn::tensor_t> &t_cost)
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
            int min_size = std::min((size_t)batch_size,
                                    inputs.size() - i);
            trainMiniBatch(inputs, &desired_outputs[i],
                       min_size, &(t_cost[i]), i);
            while( worker_done<runningWorkersNum() )
            {
                QThread::msleep(10);
                QGuiApplication::processEvents();
            }
            int nod_len = nod.size();
            // update weights
            for( int i=0 ; i<nod_len ; i++ )
            {
                nod[i]->updateWeight(&optimizer, worker_len);
            }

            emit onBatchEnumerate();
        }
        emit OnEpochEnumerate();
    }
    setNetPhase(tiny_dnn::net_phase::test);
}

void TdNetwork::setBatchSize(int bs)
{
    int ws_len = workers.size();
    batch_size = bs;

    t_batch.resize(batch_size);

    int thread_num = TD_THREAD_NUM;

    for( int i=ws_len ; i<thread_num ; i++ )
    {
        TdWorker *new_worker = new TdWorker(&nod, &t_batch,
                                            &t_cost_batch);
        QThread *thread = new QThread;
        new_worker->moveToThread(thread);
        thread->start();
        connect(new_worker, SIGNAL(finished()),
                this, SLOT(workerFinished()));
        connect(this, SIGNAL(startWorkers()),
                new_worker, SLOT(run()));
        workers.push_back(new_worker);
        workers_th.push_back(thread);
    }
}

void TdNetwork::workerFinished()
{
    worker_done++;
//    qDebug() << "worker returned" << worker_done;
}

int TdNetwork::runningWorkersNum()
{
    int len = workers.size();
    int counter = 0;
    for( int i=0 ; i<len ; i++ )
    {
        if( workers[i]->enabled )
        {
            counter++;
        }
    }
    return counter;
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

tiny_dnn::vec_t TdNetwork::predict(tiny_dnn::vec_t &first)
{
    nod.front()->setInData(first);

    TdWorker test_worker(&nod, NULL, NULL);
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

size_t TdNetwork::inDataSize()
{
    return nod.front()->inDataSize();
}

size_t TdNetwork::outDataSize()
{
    return nod.back()->outDataSize();
}

