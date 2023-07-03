#include "td_sequential.h"

TdSequential::TdSequential(QObject *parent) : QObject(parent)
{

}

void TdSequential::backward(const std::vector<tiny_dnn::tensor_t> &first)
{
    std::vector<std::vector<const tiny_dnn::vec_t *>> reordered_grad;
    reorderForLayerwiseProcessing(first, reordered_grad);
    assert(reordered_grad.size() == 1);

    nodes.back()->set_out_grads(&reordered_grad[0], 1);

    int len = nodes.size();
    for( int i=0 ; i<len ; i++ )
    {
        nodes[i]->backward();
    }
}

std::vector<tiny_dnn::tensor_t> TdSequential::forward(
        const std::vector<tiny_dnn::tensor_t> &first)
{
    std::vector<std::vector<const tiny_dnn::vec_t *>> reordered_data;
    reorderForLayerwiseProcessing(first, reordered_data);
    assert(reordered_data.size() == 1);

    nodes.front()->set_in_data(&reordered_data[0], 1);

    for( auto l:nodes )
    {
        l->forward();
    }

    std::vector<const tiny_dnn::tensor_t *> out;
    nodes.back()->output(out);

    return normalizeOut(out);
}

void TdSequential::checkConnectivity()
{
    for( size_t i=0; i<nodes.size()-1 ; i++ )
    {
        auto out = nodes[i]->outputs();
        auto in  = nodes[i + 1]->inputs();

        if( out[0]!=in[0] )
        {
            qDebug() << "ERROR TINY: in!=out";
            exit(0);
        }
    }
}

void TdSequential::updateWeights(tiny_dnn::optimizer *opt)
{
    for( auto l:nodes )
    {
        l->update_weight(opt);
    }
}

/**
 * setup all weights, must be called before forward/backward
 **/
void TdSequential::setup(bool reset_weight) {
    for( auto l:nodes )
    {
        l->setup(reset_weight);
    }
}

void TdSequential::clearGrads()
{
    for ( auto l:nodes )
    {
        l->clear_grads();
    }
}

size_t TdSequential::size()
{
    return nodes.size();
}

TdSequential::iterator TdSequential::begin()
{
    return nodes.begin();
}

TdSequential::iterator TdSequential::end()
{
    return nodes.end();
}

TdSequential::const_iterator TdSequential::begin() const
{
    return nodes.begin();
}

TdSequential::const_iterator TdSequential::end() const
{
    return nodes.end();
}

tiny_dnn::layer *TdSequential::operator[](size_t index)
{
    return nodes[index];
}

size_t TdSequential::inDataSize()
{
    return nodes.front()->in_data_size();
}

size_t TdSequential::outDataSize() const
{
    return nodes.back()->out_data_size();
}

float_t TdSequential::targetValueMin(int out_channel) const
{
    CNN_UNREFERENCED_PARAMETER(out_channel);
    return nodes.back()->out_value_range().first;
}

float_t TdSequential::targetValueMax(int out_channel) const
{
    CNN_UNREFERENCED_PARAMETER(out_channel);
    return nodes.back()->out_value_range().second;
}

void TdSequential::save(std::ostream &os)
{  // NOLINT
    for( auto &l:nodes )
    {
        l->save(os);
    }
}

void TdSequential::load(std::istream &is)
{  // NOLINT
    setup(false);
    for( auto &l:nodes )
    {
        l->load(is);
    }
}

void TdSequential::load(const std::vector<float_t> &vec)
{
    int idx = 0;
    setup(false);
    for( auto &l:nodes )
    {
        l->load(vec, idx);
    }
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

void TdSequential::label2vec(const std::vector<tiny_dnn::label_t> &labels,
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

template<typename T>
void TdSequential::add(T &&layer)
{
    pushBack(std::forward<T>(layer));

    if( nodes.size()!=1 )
    {
        auto head = nodes[nodes.size() - 2];
        auto tail = nodes[nodes.size() - 1];
        tiny_dnn::connect(head, tail, 0, 0);
        auto out = head->outputs();
        auto in  = tail->inputs();
    }
    checkConnectivity();
}

template<typename InputArchive>
void TdSequential::loadConnections(InputArchive &ia)
{
    CNN_UNREFERENCED_PARAMETER(ia);
    for( size_t i=0 ; i<nodes.size()-1 ; i++ )
    {
        auto head = nodes[i];
        auto tail = nodes[i + 1];
        tiny_dnn::connect(head, tail, 0, 0);
    }
}

template <typename OutputArchive>
void TdSequential::saveModel(OutputArchive &oa)
{
  oa(cereal::make_nvp("TdSequential", nodes));
  dynamic_cast<const TdSequential *>(this)->saveConnections(oa);
}

template <typename InputArchive>
void TdSequential::loadModel(InputArchive &ia)
{
  own_nodes.clear();
  nodes.clear();

  ia(cereal::make_nvp("TdSequential", own_nodes));

  for( auto &n:own_nodes )
  {
    nodes.push_back(&*n);
  }

  dynamic_cast<TdSequential *>(this)->loadConnections(ia);
}

template<typename OutputArchive>
void TdSequential::saveConnections(OutputArchive &)
{
    // empty!
}

template<typename T> T &TdSequential::at(size_t index)
{
    const T *v = dynamic_cast<const T *>(nodes[index]);
    if( v )
    {
        return *v;
    }
    qDebug() << "failed to cast";
    exit(0);
}

template<typename OutputArchive>
void TdSequential::saveWeights(OutputArchive &oa)
{
    for( auto n:nodes )
    {
        oa(*n);
    }
}

template<typename InputArchive>
void TdSequential::loadWeights(InputArchive &ia)
{
    for( auto n:nodes )
    {
        ia(*n);
    }
}

template<typename T> void TdSequential::pushBack(T &&node)
{ // NOLINT
    pushBackImpl(
        std::forward<T>(node),
        typename std::is_rvalue_reference<decltype(node)>::type());
}

template<typename T>
void TdSequential::pushBack(std::shared_ptr<T> node)
{
    own_nodes.push_back(node);
    nodes.push_back(own_nodes.back().get());
}

template<typename T>
void TdSequential::pushBackImpl(T &&node, std::true_type)
{  // is_rvalue_reference
    own_nodes.push_back(
        std::make_shared<typename std::remove_reference<T>::type>(
            std::forward<T>(node)));
    nodes.push_back(own_nodes.back().get());
}

template<typename T>
void TdSequential::pushBackImpl(T &&node, std::false_type)
{
    nodes.push_back(&node);
}
