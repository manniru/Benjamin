#include "td_mini_worker.h"

TdMiniWorker::TdMiniWorker(std::vector<TdLayer *> *nodes,
                   std::vector<int> *o,
                   std::vector<float> *t_co,
                   QObject *parent) :
    QObject(parent)
{
    nod = nodes;
    outputs = o;
    costs = t_co;
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
    start_time = clock();
    o_batch = forward();
    QString fw_time = getDiffTime(start_time); start_time = clock();
    // back propagation
    std::vector<tiny_dnn::tensor_t> delta;
    delta = calcGrad(o_batch);
    // backward
    nod->back()->setOutGrads(delta, s_index, e_index);
    QString grad_time = getDiffTime(start_time); start_time = clock();

    for( int j=nod_len ; j>0 ; j-- )
    {
        (*nod)[j-1]->backward(s_index, e_index);
    }
    QString bw_time = getDiffTime(start_time); start_time = clock();
    qDebug() << s_index << e_index << "fw:" << fw_time
             << "|grad:" << grad_time << "|bw:" << bw_time;
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
    TdLayer *back = nod->back();
    /// FIXME: if out_type[i]==tiny_dnn::vector_type::data
    out.push_back(&back->out_edges->data_);

    return td_normalizeOut(out);
}

std::vector<tiny_dnn::tensor_t> TdMiniWorker::calcGrad(
       std::vector<tiny_dnn::tensor_t> &y)
{
    const size_t sample_count  = y.size();
    std::vector<tiny_dnn::tensor_t> gradients(sample_count);

    for( int sample=s_index ; sample<e_index ; sample++ )
    {
        gradients[sample].resize(y[sample].size());
        gradients[sample][0] = mse_df(y[sample][0],
                                      (*outputs)[sample]);
        applyCost(gradients[sample][0]);
    }

    return gradients;
}

void TdMiniWorker::applyCost(tiny_dnn::vec_t &gradient)
{
    int class_len = gradient.size();
    for( int i=0 ; i<class_len ; i++ )
    {
        gradient[i] *= (*costs)[i];
    }
}

tiny_dnn::vec_t TdMiniWorker::mse_df(tiny_dnn::vec_t &y,
                                     float o)
{
    tiny_dnn::vec_t d(y.size());
    float_t factor = float_t(2) / static_cast<float_t>(y.size());

    for(size_t i = 0; i < y.size(); ++i)
    {
        if( o==i )
        {
            d[i] = factor * (y[i] - 1);
        }
        else
        {
            d[i] = factor * y[i];
        }
    }

    return d;
}
