#include "td_sequential.h"

TdSequential::TdSequential(QObject *parent) : QObject(parent)
{

}

void TdSequential::backward(
        const std::vector<tiny_dnn::tensor_t> &first)
{
    qDebug() << "BACKWARD";
    std::vector<std::vector<const tiny_dnn::vec_t *>> reordered_grad;
    reorderForLayerwiseProcessing(first, reordered_grad);
    assert(reordered_grad.size() == 1);

    nod.back()->set_out_grads(&reordered_grad[0], 1);

    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->backward();
    }
}

std::vector<tiny_dnn::tensor_t> TdSequential::forward(
        const std::vector<tiny_dnn::tensor_t> &first)
{
    qDebug() << "FORWARD";
    std::vector<std::vector<const tiny_dnn::vec_t *>> reordered_data;
    reorderForLayerwiseProcessing(first, reordered_data);
    assert(reordered_data.size() == 1);

    nod.front()->set_in_data(&reordered_data[0], 1);

    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->forward();
    }

    std::vector<const tiny_dnn::tensor_t *> out;
    nod.back()->output(out);

    return normalizeOut(out);
}

void TdSequential::checkConnectivity()
{
    int len = nod.size();
    for( int i=0; i<len-1 ; i++ )
    {
        auto out = nod[i]->outputs();
        auto in  = nod[i+1]->inputs();

        if( out[0]!=in[0] )
        {
            qDebug() << "ERROR TINY: in!=out";
            exit(0);
        }
    }
}

void TdSequential::updateWeights(tiny_dnn::optimizer *opt)
{
    qDebug() << "WEIGHT";
    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->update_weight(opt);
    }
}

/**
 * setup all weights, must be called before forward/backward
 **/
void TdSequential::setup(bool reset_weight)
{
    qDebug() << "SETUP";
    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->setup(reset_weight);
    }
}

void TdSequential::clearGrads()
{
    qDebug() << "CLEAR";
    int len = nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        nod[i]->clear_grads();
    }
}

size_t TdSequential::inDataSize()
{
    return nod.front()->in_data_size();
}

size_t TdSequential::outDataSize() const
{
    return nod.back()->out_data_size();
}

float_t TdSequential::targetValueMin(int out_channel) const
{
    CNN_UNREFERENCED_PARAMETER(out_channel);
    return nod.back()->out_value_range().first;
}

float_t TdSequential::targetValueMax(int out_channel) const
{
    CNN_UNREFERENCED_PARAMETER(out_channel);
    return nod.back()->out_value_range().second;
}

void TdSequential::label2vec(const tiny_dnn::label_t *t, size_t num,
                             std::vector<tiny_dnn::vec_t> &vec)
{
    size_t outdim = outDataSize();

    vec.reserve(num);
    for( size_t i=0 ; i<num ; i++ )
    {
        assert(t[i] < outdim);
        vec.emplace_back(outdim, targetValueMin());
        vec.back()[t[i]] = targetValueMax();
    }
}

void TdSequential::label2vec(
        const std::vector<tiny_dnn::label_t> &labels,
        std::vector<tiny_dnn::vec_t> &vec)
{
    return label2vec(&labels[0], labels.size(), vec);
}

void TdSequential::reorderForLayerwiseProcessing(
        const std::vector<tiny_dnn::tensor_t> &input,
        std::vector<std::vector<const tiny_dnn::vec_t *> > &output)
{
    size_t sample_count  = input.size();
    size_t channel_count = input[0].size();

    output.resize(channel_count);
    for( size_t i=0 ; i<channel_count ; ++i )
    {
        output[i].resize(sample_count);
    }

    for( size_t sample=0 ; sample<sample_count ; ++sample )
    {
        assert(input[sample].size() == channel_count);
        for( size_t channel=0 ; channel<channel_count ; ++channel )
        {
            output[channel][sample] = &input[sample][channel];
        }
    }
}

std::vector<tiny_dnn::tensor_t> TdSequential::normalizeOut(
        const std::vector<const tiny_dnn::tensor_t *> &out)
{
    // normalize indexing back to [sample][layer][feature]
    std::vector<tiny_dnn::tensor_t> normalized_output;

    const size_t sample_count = out[0]->size();
    normalized_output.resize(sample_count, tiny_dnn::tensor_t(1));

    for( size_t sample=0; sample<sample_count ; ++sample )
    {
        normalized_output[sample][0] = (*out[0])[sample];
    }

    return normalized_output;
}

void TdSequential::add(tiny_dnn::layer *layer)
{
    nod.push_back(layer);
    connectHeadToTail();
}

void TdSequential::connectHeadToTail()
{
    if( nod.size()!=1 )
    {
        auto head = nod[nod.size() - 2];
        auto tail = nod[nod.size() - 1];
        tiny_dnn::connect(head, tail, 0, 0);
        auto out = head->outputs();
        auto in  = tail->inputs();
    }
    checkConnectivity();
}
