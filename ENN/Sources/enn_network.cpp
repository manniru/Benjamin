#include "enn_network.h"
#include "tiny_dnn/util/target_cost.h"

timer nn_t;
int nn_epoch;

EnnNetwork::EnnNetwork(QString word, int id,
                       QObject *parent) : QObject(parent)
{
    dataset = new EnnDataset(word, id);
    n_train_epochs = ENN_MAX_EPOCH;
    net_state = ENN_NETWORK_NORMAL;

    setbuf(stdout,NULL); //to work out printf

    net = new TdNetwork(ENN_BATCH_SIZE);
    parser = new EnnParse(net);
    connect(net, SIGNAL(onEpochEnumerate()), this, SLOT(epochLog()));
    need_train = load();
}

EnnNetwork::~EnnNetwork()
{
    delete dataset;
}

void EnnNetwork::benchmark()
{
    tiny_dnn::timer t;  // start the timer

    // predict
    net->predict(dataset->test_datas[0]);

    double elapsed_s = t.elapsed();
    t.stop();
    qDebug() << dataset->m_name << "Finished! single infere time:"
             << elapsed_s;
}

void EnnNetwork::save()
{
    QDir au_OnlineDir("Models");

    if( !au_OnlineDir.exists() )
    {
        qDebug() << "Models";
        system("mkdir -p Models");
    }

    QString mdl_path = "Models/";
    mdl_path += dataset->m_name;
    mdl_path += ".enn";
    parser->save(mdl_path);
}

// return true if need training
bool EnnNetwork::load()
{
    QString model_path = "Models/";
    model_path += dataset->m_name;
    model_path += ".enn";
    if( QFile::exists(model_path) )
    {
        // load model
        parser->load(model_path);

        // chack the loss
        float loss = calcLoss();
        if( loss<ENN_TARGET_LOSS )
        {
            return false;
        }
        if( net_state==ENN_NETWORK_NEGATIVE )
        {
            return false; //model contain non converge sample
        }
        else if( net_state==ENN_NETWORK_LOCKED )
        {
            net = new TdNetwork(ENN_BATCH_SIZE); // reset network
            parser->net = net;
            createNNet();
            qDebug() << "Reset from scratch";
            net_state = ENN_NETWORK_NORMAL;
            return true;
        }
        qDebug() << "model " << dataset->model_id
                 << dataset->m_name
                 << "loaded" << "loss"   << loss;
    }
    else // create new
    {
        createNNet();
    }
    return true;
}

void EnnNetwork::createNNet()
{
#ifdef ENN_IMAGE_DATASET
    net << conv(40, 40, 5, 40, 3, 10)      << activation::leaky_relu() // 40x5 kernel, 3 channel, 10 filter
        << ave_pool(36, 1, 10, 2, 1, 2, 1) << activation::leaky_relu() // pool 2x1, stride 2x1
        << conv(18, 1, 3, 1, 10, 20)       << activation::leaky_relu()
        << ave_pool(16, 1, 20, 2, 1, 2, 1) << activation::leaky_relu()
        << conv(8, 1, 8, 1, 20, 60)        << activation::leaky_relu() // flatten conv
        << fc(60, 2)                       << activation::softmax();
#else
//    int f1 = 20;
//    int f2 = f1 + 10;
//    net->addConv(40, 40, 5, 40, 1, f1)      ->addLeakyRelu();
//    net->addAvePool(36, 1, f1, 2, 1, 2, 1)  ->addLeakyRelu();
//    net->addConv(18, 1, 3, 1, f1, f2)       ->addLeakyRelu();
//    net->addAvePool(16, 1, f2, 2, 1, 2, 1)  ->addLeakyRelu();
//    net->addConv(8, 1, 8, 1, f2, 60)        ->addLeakyRelu();
//    net->addFC(60, 2)                       ->addSoftMax();

    QString network;
    network += "Conv(40, 40, 5, 40, 1, 20)\nLeakyRelu\n";
    network += "AvePool(36, 1, 20, 2, 1, 2, 1)\nLeakyRelu\n";
    network += "Conv(18, 1, 3, 1, 20, 30)\nLeakyRelu\n";
    network += "AvePool(16, 1, 30, 2, 1, 2, 1)\nLeakyRelu\n";
    network += "Conv(8, 1, 8, 1, 30, 60)\nLeakyRelu\n";
    network += "FC(60, 2)\nSoftMax";
#endif
    parser->parseNetwork(network);
    last_loss = 9999;
}

void EnnNetwork::train(float l_rate)
{
    qDebug() << "dataset size: test"  << dataset->test_datas.size()
             << "train" << dataset->train_datas.size()
             << "l_rate" << l_rate;

    if( !need_train )
    {
        return;
    }

    net->worker->optimizer.alpha = l_rate; // learning rate = 1E-4
    nn_epoch = 0;

    std::vector<vec_t> target_cost;
    target_cost = create_balanced_target_cost(dataset->train_labels);

    std::vector<tensor_t> output_tensor;
    net->normalizeTensor(dataset->train_labels, output_tensor);

    net->fit(dataset->train_datas, output_tensor, n_train_epochs, false,
                 target_cost);

    if( net_state!=ENN_NETWORK_LOCKED )
    {
        save();
    }
    benchmark();
}

vec_t EnnNetwork::test(vec_t *data)
{
    vec_t res = net->predict(*data);
    return res;
}

void EnnNetwork::epochLog()
{
    nn_epoch++;
    if( nn_epoch%ENN_EPOCH_LOG==0 )
    {
        QString t_elapsed = QString::number(nn_t.elapsed());
        t_elapsed += "s";
        float loss = calcLoss();
        EnnResult acc_test = getAccuracy(dataset->test_datas,
                                  dataset->test_labels);
        EnnResult acc_train = getAccuracy(dataset->train_datas,
                                  dataset->train_labels);

        qDebug() << nn_epoch << t_elapsed
                 << acc_train.msg << "test" << acc_test.msg << "loss:"
                 << loss << "alpha"
                 << net->worker->optimizer.alpha*1000;

        if( loss<ENN_TARGET_LOSS )
        {
            net->stopOngoingTraining();
        }

        net->worker->optimizer.alpha *= 0.95;
        nn_t.restart();
    }
}

// return loss
float EnnNetwork::calcLoss()
{
    // calc loss
    float loss = 0;

    std::vector<tensor_t> label_tensor;
    net->normalizeTensor(dataset->train_labels, label_tensor);
    int len = dataset->train_datas.size();

    float          wrong_sum;
    QVector<int>   wrong_i;
    QVector<float> wrong_loss;
    wrong_sum = 0;
    for( int i=0 ; i<len ; i++ )
    {
        vec_t predicted = net->predict(dataset->train_datas[i]);
        float s_loss = mse::f(predicted, label_tensor[i][0]);

        if( s_loss>ENN_WRONG_LOSS )
        {
            wrong_sum += s_loss;
            wrong_i.push_back(i);
            wrong_loss.push_back(s_loss);
        }
        loss += s_loss;
    }
    if( wrong_sum>0 )
    {
        float diff = loss - wrong_sum;
        last_loss = diff;
        handleWrongs(diff, wrong_i, wrong_loss);
    }
    else
    {
        last_loss = loss;
    }
    if( loss>ENN_EXPLODE_LOSS || std::isnan(loss) )
    {
        EnnResult acc_train = getAccuracy(dataset->train_datas,
                                  dataset->train_labels);
        double learned_percent = acc_train.det_t*100;
        learned_percent = learned_percent/acc_train.tot_t;
        qDebug() << "learned_percent"
                 << learned_percent;
        if( learned_percent<10 )
        {
            qDebug() << "========= NO CONVERGANCE"
                     << dataset->m_name
                     << " ==========";
            net->stopOngoingTraining();
            net_state = ENN_NETWORK_LOCKED;
        }
    }

    return loss;
}

EnnResult EnnNetwork::getAccuracy(std::vector<vec_t>   &data,
                           std::vector<label_t> &label)
{
    EnnResult res;
    res.msg = "T[";

    int len = data.size();
    for( int i=0 ; i<len ; i++ )
    {
        vec_t out = test(&(data[i]));
        if( label[i]==1 )
        {
            res.tot_t++;
            if( out[1]>0.6 )
            {
                res.det_t++;
            }
        }
        else
        {
            res.tot_f++;
            if( out[0]>0.6 )
            {
                res.det_f++;
            }
        }
    }

    res.msg += QString::number(res.det_t) + "/";
    res.msg += QString::number(res.tot_t) + "] F[";

    res.msg += QString::number(res.det_f) + "/";
    res.msg += QString::number(res.tot_f) + "] Tot[";

    res.msg += QString::number(res.det_t+res.det_f) + "/";
    res.msg += QString::number(len) + "]";

    return res;
}

void EnnNetwork::handleWrongs(float diff, QVector<int> &wrong_i,
                  QVector<float> &wrong_loss)
{
    for( int i=0 ; i<wrong_i.length() ; i++ )
    {
        QString path = dataset->train_path[wrong_i[i]];
        int label = dataset->train_labels[wrong_i[i]];
        if( path.contains(dataset->m_name) )
        {
            printf("wr %5.5s diff=%.2f label=%d loss=%.2f"
                   " i=%4d %s\n",
                   dataset->m_name.toStdString().c_str(),
                   diff, label, wrong_loss[i], wrong_i[i],
                   path.toStdString().c_str());
        }
        else
        {
            printf("2w %5.5s diff=%.2f label=%d loss=%.2f"
                   " i=%4d %s\n",
                   dataset->m_name.toStdString().c_str(),
                   diff, label, wrong_loss[i], wrong_i[i],
                   path.toStdString().c_str());
        }

        if( label==1 )
        {
            if( !(path.contains(dataset->m_name)) )
            {
                qDebug() << "What The FUCK";
            }
        }
    }
    if( diff<ENN_TARGET_LOSS && wrong_i.length()<5 )
    {
        net->stopOngoingTraining();
        net_state = ENN_NETWORK_NEGATIVE;

        EnnResult acc_test = getAccuracy(dataset->test_datas,
                                  dataset->test_labels);
        EnnResult acc_train = getAccuracy(dataset->train_datas,
                                  dataset->train_labels);

        qDebug() << "train" << acc_train.msg
                 << "test" << acc_test.msg
                 << "loss(diff)" << diff;
    }
}

