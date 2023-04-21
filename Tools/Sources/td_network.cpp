#include "tiny_dnn/tiny_dnn.h"
#include <fstream>

using namespace tiny_dnn;

TdNetwork::TdNetwork(QString word)
{
    m_name = word;
    model_loaded = false;

    QString model_path = "../ENN/Models/";
    model_path += m_name;
    model_path += ".mdl";
    if( QFile::exists(model_path) )
    {
        // load model
        std::ifstream ifs(model_path.toStdString().c_str(),
                          std::ios::binary | std::ios::in);
        if( ifs.fail() || ifs.bad())
        {
            qDebug() << "model " << model_path << "failed to open";
            return;
        }

        cereal::BinaryInputArchive bi(ifs);
        load(bi);
//        qDebug() << "model " << model_path << "loaded";
        model_loaded = true;
    }
    else
    {
        qDebug() << "model " << model_path << "doesn't exist";
    }
}

TdNetwork::~TdNetwork()
{
}

vec_t TdNetwork::predict(float *data, int len)
{
    layers.front()->set_in_data(data, len);

    int l_len = layers.size();
    for( int i=0 ; i<l_len ; i++ )
    {
        layers[i]->forward();
    }

    std::vector<const tensor_t *> out;
    layers.back()->output(out);

    return (*out[0])[0];
}

void TdNetwork::load_connections()
{
    int len = layers.size()-1;
    for( int i=0; i<len ; i++ )
    {
        tiny_dnn::layer *head = layers[i];
        tiny_dnn::layer *tail = layers[i + 1];
        tiny_dnn::connect(head, tail, 0, 0);
    }
}
