#include "bt_network.h"

using namespace tiny_dnn;

BtNetwork::BtNetwork()
{
    word_list = bt_parseLexicon(BT_WORDS_PATH);

    int len = word_list.size();
    nets.resize(len);
    for( int i=0 ; i<len ; i++ )
    {
        nets[i] = new TdNetwork(word_list[i]);
    }
}

BtNetwork::~BtNetwork()
{
    int len = nets.size();
    for( int i=0 ; i<len ; i++ )
    {
        delete nets[i];
    }
}

float BtNetwork::predict(int id, float *data)
{
    if( nets[id]->model_loaded )
    {
        int data_len = BT_ENN_SIZE*BT_ENN_SIZE*3;
        vec_t res = nets[id]->predict(data, data_len);
        qDebug() << word_list[id]
                 << "Detect:" << res[0] << res[1];

        return res[0];
    }
    return 0;
}
