#include "enn_parse.h"
#include <QFile>
#include <QDataStream>

EnnParse::EnnParse(TdNetwork *n, QObject *parent) :
    QObject(parent)
{
    net = n;
}

void EnnParse::parseNetwork(QString network)
{
    net_str = network;
    QStringList layers_str = network.split("\n");
    int layers_len = layers_str.length();
    EnnLayerSpec layer;

    for( int i=0 ; i<layers_len ; i++ )
    {
        layer = parseLayer(layers_str[i].trimmed());
        addToNetwork(layer);
    }
    net->initWeightBias();
}

EnnLayerSpec EnnParse::parseLayer(QString data)
{
    EnnLayerSpec ret;
    if( data.contains("(") )
    {
        data.chop(1); // remove ")"
        QStringList layer_str = data.split("(", Qt::SkipEmptyParts);
        ret.layer_name = layer_str[0].trimmed();

        QString args_str = layer_str[1].trimmed();
        QStringList args = args_str.split(",");
        int args_len = args.size();
        for( int i=0 ; i<args_len ; i++ )
        {
            QString arg = args[i].trimmed();
            ret.specs.push_back(arg.toInt());
        }
    }
    else
    {
        ret.layer_name = data;
    }
    return ret;
}

void EnnParse::addToNetwork(EnnLayerSpec layer)
{
    if( layer.layer_name==CONV_LAYER_STR )
    {
        if( layer.specs.size()==6 )
        {
            int width = layer.specs[0];
            int height = layer.specs[1];
            int window_width = layer.specs[2];
            int window_height = layer.specs[3];
            int in_channel = layer.specs[4];
            int out_channel = layer.specs[5];
            net->addConv(width, height, window_width, window_height,
                         in_channel, out_channel);
        }
        else
        {
            qDebug() << "Error 88: wrong input for" << CONV_LAYER_STR;
        }
    }
    else if( layer.layer_name==FC_LAYER_STR )
    {
        if( layer.specs.size() )
        {
            int in_dim = layer.specs[0];
            int out_dim = layer.specs[1];
            net->addFC(in_dim, out_dim);
        }
        else
        {
            qDebug() << "Error 88: wrong input for" << FC_LAYER_STR;
        }
    }
    else if( layer.layer_name==AVERAGE_POOLING_STR )
    {
        if( layer.specs.size() )
        {
            int width = layer.specs[0];
            int height = layer.specs[1];
            int in_channel = layer.specs[2];
            int pool_size_x = layer.specs[3];
            int pool_size_y = layer.specs[4];
            int stride_x = layer.specs[5];
            int stride_y = layer.specs[6];
            net->addAvePool(width, height, in_channel, pool_size_x,
                            pool_size_y, stride_x, stride_y);
        }
        else
        {
            qDebug() << "Error 88: wrong input for"
                     << AVERAGE_POOLING_STR;
        }
    }
    else if( layer.layer_name==LEAKY_RELU_STR )
    {
        net->addLeakyRelu();
    }
    else if( layer.layer_name==SOFTMAX_LAYER_STR )
    {
        net->addSoftMax();
    }
    else
    {
        qDebug() << "Error 88: wrong layer name" << layer.layer_name;
    }
}

void EnnParse::save(QString filename)
{
    QFile model_file(filename);

    if( !model_file.open(QIODevice::WriteOnly) )
    {
        qDebug() << "Error opening" << filename;
        return;
    }
    QDataStream out(&model_file);

    // save network structure
    out << net_str;

    // save network weights and bias
    int len = net->nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        if( net->nod[i]->trainable==false ) // skip activation layers
        {
            continue;
        }
        tiny_dnn::vec_t weights = net->nod[i]->in_edges[1]->data_[0];
        int w_len = weights.size();
        QVector<float> w_f(w_len);
        for( int j=0 ; j<w_len ; j++ )
        {
            w_f[j] = weights[j];
        }
        out << w_f;

        tiny_dnn::vec_t bias = net->nod[i]->in_edges[2]->data_[0];
        int b_len = bias.size();
        QVector<float> b_f(w_len);
        for( int j=0 ; j<b_len ; j++ )
        {
            b_f[j] = bias[j];
        }
        out << b_f;
    }

    model_file.close();
}

void EnnParse::load(QString filename)
{
    QFile model_file(filename);

    if( !model_file.open(QIODevice::ReadOnly) )
    {
        qDebug() << "Error opening" << filename;
        exit(0);
    }
    QDataStream in(&model_file);

    // save network structure
    in >> net_str;
    parseNetwork(net_str);

    // save network weights and bias
    int len = net->nod.size();
    for( int i=0 ; i<len ; i++ )
    {
        if( !net->nod[i]->trainable ) // skip activation layers
        {
            continue;
        }
        tiny_dnn::vec_t *weights = &net->nod[i]->in_edges[1]->data_[0];
        int w_len = weights->size();
        QVector<float> w_f;
        in >> w_f;
        for( int j=0 ; j<w_len ; j++ )
        {
            (*weights)[j] = w_f[j];
        }

        tiny_dnn::vec_t *bias = &net->nod[i]->in_edges[2]->data_[0];
        int b_len = bias->size();
        QVector<float> b_f(w_len);
        in >> b_f;
        for( int j=0 ; j<b_len ; j++ )
        {
            (*bias)[j] = b_f[j];
        }
    }

    model_file.close();
}
