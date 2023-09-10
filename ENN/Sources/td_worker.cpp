#include "td_worker.h"

TdWorker::TdWorker(std::vector<TdLayer *> *nodes,
                   QObject *parent) : QObject(parent)
{
    nod = nodes;
}

void TdWorker::setBatchSize(int bs)
{
    int ws_len = workers.size();
    batch_size = bs;

    o_batch.resize(batch_size);

    int thread_num = TD_THREAD_NUM;

    for( int i=ws_len ; i<thread_num ; i++ )
    {
        TdMiniWorker *new_worker = new TdMiniWorker(nod, &o_batch,
                                            &cost);
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

void TdWorker::trainMiniBatch(int data_size, int offset)
{
    std::copy(&(*outputs)[batch_cnt],
              &(*outputs)[batch_cnt] + data_size, &o_batch[0]);
    nod->front()->setInData(*in_vec, data_size, offset);

    int nod_len = nod->size();
    for( int i=0 ; i<nod_len ; i++ )
    {
        // resize outs and stuff to have room for every input sample in
        // the batch
        (*nod)[i]->set_sample_count(data_size);
        tiny_dnn::tensor_t &dst_grad = (*nod)[i]->out_edges->grad_;
        dst_grad.resize(data_size);
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
    start_time = clock();
    emit startMiniWorkers();
}

void TdWorker::fit(tiny_dnn::tensor_t *inputs,
        std::vector<int> *desired_outputs,
        int epoch)
{
    stop_training = false;
    optimizer.reset();

    iter_cnt = 0;
    total_epoch = epoch;
    in_vec = inputs;
    outputs = desired_outputs;
    calcCost();
    trainEpoch();
}

void TdWorker::trainEpoch()
{
    if( iter_cnt<total_epoch && !stop_training )
    {
        batch_cnt = 0;
        trainBatch();
    }
    else
    {
        emit epochFinished();
    }
}

void TdWorker::trainBatch()
{
    int len_input = in_vec->size();
    if( batch_cnt<len_input && !stop_training )
    {
        int min_size = std::min((size_t)batch_size,
                                in_vec->size() - batch_cnt);
        trainMiniBatch(min_size, batch_cnt);
    }
    else
    {
        emit onEpochEnumerate();
        iter_cnt++;
        trainEpoch();
    }
}

void TdWorker::miniWorkerFinished()
{
    worker_done++;
//    qDebug() << worker_done << "worker returned";
    if( worker_done<runningWorkersNum() )
    {
        return;
    }
    QString fw_grad_back_time = getDiffTime(start_time);
    start_time = clock();
    int nod_len = nod->size();
    // update weights
    for( int i=0 ; i<nod_len ; i++ )
    {
        (*nod)[i]->updateWeight(&optimizer, worker_len);
    }
    QString update_w_time = getDiffTime(start_time);
    qDebug() << "fw_grad_back_time" << fw_grad_back_time
             << "update_w_time" << update_w_time;
    batch_cnt += batch_size;
    trainBatch();
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

void TdWorker::calcCost()
{
    std::vector<size_t> label_counts;
    int len = outputs->size();
    int max_label = 0;
    for( int i=0 ; i<len ; i++ )
    {
        if( (*outputs)[i]>max_label )
        {
            max_label = (*outputs)[i];
        }
    }
    label_counts.resize(max_label+1, 0);
    for( int i=0 ; i<len ; i++ )
    {
        label_counts[(*outputs)[i]]++;
    }

    size_t total_sample_count = outputs->size();
    int class_count = label_counts.size();
    cost.resize(class_count);

    for( int i=0 ; i<class_count ; i++ )
    {
        float weight = (float)total_sample_count/
                (class_count*label_counts[i]);
        cost[i] = weight;
    }
}
