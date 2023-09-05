#include "td_nodes.h"

// transform indexing so that it's more suitable for per-layer operations
// input:  [sample][channel][feature]
// output: [channel][sample][feature]
void td_reorder(
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

std::vector<tiny_dnn::tensor_t> td_normalizeOut(
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

void td_isSizeMatch(std::vector<TdLayer *> *nod)
{
    int len = nod->size();
    for( int i=0; i<len-1 ; i++ )
    {
        TdEdge *out = (*nod)[i]->out_edges;
        TdEdge *in  = (*nod)[i+1]->in_edges[0];

        if( out!=in )
        {
            qDebug() << "ERROR 44: size mismatch"
                     << out->shape_ << in->shape_;
            exit(0);
        }
    }
}

void td_connectHeadToTail(std::vector<TdLayer *> *nod)
{
    if( nod->size()!=1 )
    {
        TdLayer *head = (*nod)[nod->size() - 2];
        TdLayer *tail = (*nod)[nod->size() - 1];
        td_connectLayer(head, tail);
    }
    td_isSizeMatch(nod);
}
