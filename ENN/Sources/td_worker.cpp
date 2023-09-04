#include "td_worker.h"
#include <QGuiApplication>

TdWorker::TdWorker(std::vector<TdLayer *> *nodes,
                   QObject *parent) : QObject(parent)
{
    nod = nodes;
}

void TdWorker::setBatchSize(int bs)
{
    int ws_len = workers.size();
    batch_size = bs;

    t_batch.resize(batch_size);

    int thread_num = TD_THREAD_NUM;

    for( int i=ws_len ; i<thread_num ; i++ )
    {
        TdMiniWorker *new_worker = new TdMiniWorker(nod, &t_batch,
                                            &t_cost_batch);
        QThread *thread = new QThread;
        new_worker->moveToThread(thread);
        thread->start();
        connect(new_worker, SIGNAL(finished()),
                this, SLOT(miniWorkerFinished()));
        connect(this, SIGNAL(startMiniWorkers()),
                new_worker, SLOT(run()));
        workers.push_back(new_worker);
        workers_th.push_back(thread);
    }
}

void TdWorker::miniWorkerFinished()
{
    worker_done++;
//    qDebug() << "worker returned" << worker_done;
}

/**
* train on one minibatch
* @param size is the number of data points to use in this batch
*/
void TdWorker::trainMiniBatch(std::vector<tiny_dnn::tensor_t> &in,
                           tiny_dnn::tensor_t *t,
                           int data_size,
                           tiny_dnn::tensor_t *t_cost,
                           int offset)
{
    std::copy(&t[0], &t[0] + data_size, &t_batch[0]);
    t_cost_batch = std::vector<tiny_dnn::tensor_t>(&t_cost[0],
        &t_cost[0] + data_size);
    nod->front()->setInData(in, data_size, offset);

    int nod_len = nod->size();
    for( int i=0 ; i<nod_len ; i++ )
    {
        // resize outs and stuff to have room for every input sample in
        // the batch
        (*nod)[i]->set_sample_count(data_size);
        int ch_len = (*nod)[i]->out_channels;
        for(int j=0 ; j<ch_len ; j++ )
        {
            tiny_dnn::tensor_t &dst_grad = (*nod)[i]->ith_out_node(j)->grad_;
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
    emit startMiniWorkers();
}

void TdWorker::trainEpoch(std::vector<tiny_dnn::tensor_t> &inputs,
                    std::vector<tiny_dnn::tensor_t> &desired_outputs,
                    int epoch, std::vector<tiny_dnn::tensor_t> &t_cost)
{
    stop_training = false;
    int len_input = inputs.size();
    optimizer.reset();

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
            int nod_len = nod->size();
            // update weights
            for( int i=0 ; i<nod_len ; i++ )
            {
                (*nod)[i]->updateWeight(&optimizer, worker_len);
            }
        }
        emit OnEpochEnumerate();
    }
}

int TdWorker::runningWorkersNum()
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
