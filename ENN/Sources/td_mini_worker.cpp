#include "td_mini_worker.h"

TdMiniWorker::TdMiniWorker(std::vector<TdLayer *> *nodes,
                   std::vector<tiny_dnn::tensor_t> *t_bat,
                   std::vector<tiny_dnn::tensor_t> *t_cobat,
                   QObject *parent) :
    QObject(parent)
{
    nod = nodes;
    t_batch = t_bat;
    t_cost_batch = t_cobat;
    enabled = 1;
}

void TdMiniWorker::setRange(int s_id, int e_id)
{
    s_index = s_id;
    e_index = e_id;
}

void TdMiniWorker::run()
{
    if( enabled==0 )
    {
        return;
    }

    int nod_len = nod->size();
    std::vector<tiny_dnn::tensor_t> o_batch;
//    qDebug() << "forward" << s_index << e_index;
    o_batch = forward();
    // back propagation
    std::vector<tiny_dnn::tensor_t> delta;
//    qDebug() << "gradient" << s_index << e_index;
    delta = tiny_dnn::gradient<tiny_dnn::mse>(o_batch, *t_batch,
              *t_cost_batch, s_index, e_index);
    // backward
//    qDebug() << "setOutGrads" << s_index << e_index;
    nod->back()->setOutGrads(delta, s_index, e_index);

//    qDebug() << "backward" << s_index << e_index;
    for( int j=nod_len ; j>0 ; j-- )
    {
        (*nod)[j-1]->backward(s_index, e_index);
    }
//    qDebug() << "finished" << s_index << e_index;
    emit finished();
}

std::vector<tiny_dnn::tensor_t> TdMiniWorker::forward()
{
    int len = nod->size();
    for( int i=0 ; i<len ; i++ )
    {
//        qDebug() << "layer" << i << ">>" << s_index << e_index;
        (*nod)[i]->forward(s_index, e_index);
    }

    std::vector<const tiny_dnn::tensor_t *> out;
    nod->back()->output(out);

    return td_normalizeOut(out);
}
