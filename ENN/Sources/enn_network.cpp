#include "enn_network.h"

using namespace tiny_dnn::activation;
using namespace tiny_dnn::layers;

timer nn_t;
int nn_epoch;

EnnNetwork::EnnNetwork(QString word)
{
    dataset = new EnnDataset(word);
    n_minibatch = 16;
    n_train_epochs = ENN_MAX_EPOCH;
    is_wrong = 0;
}

EnnNetwork::~EnnNetwork()
{
    delete dataset;
}

void EnnNetwork::benchmark()
{
    tiny_dnn::timer t;  // start the timer

    // predict
    vec_t res = net.predict(dataset->test_images[0]);

    double elapsed_s = t.elapsed();
    t.stop();
    qDebug() << dataset->m_name << "Finished!"
             << elapsed_s;
    qDebug() << "Detect:" << res[0] << res[1];
    qDebug() << "Detect:" << dataset->test_labels[0];
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
    mdl_path += ".mdl";
    net.save(mdl_path.toStdString().c_str());
}

// return true if need training
bool EnnNetwork::load()
{
    QString model_path = "Models/";
    model_path += dataset->m_name;
    model_path += ".mdl";
    if( QFile::exists(model_path) )
    {
        // load model
        net.load(model_path.toStdString());

        // chack the loss
        float loss = calcLoss();
        if( loss<ENN_TARGET_LOSS )
        {
            return false;
        }
        if( is_wrong==1 )
        {
            return false; //model contain non converge sample
        }
        else if( is_wrong==2 )
        {
            net = network<sequential>(); // reset network
            createNNet();
            qDebug() << "Reset from scratch";
            is_wrong = 0;
            return true;
        }
        qDebug() << "model " << model_path << "loaded";
    }
    else // create new
    {
        createNNet();
    }
    return true;
}

void EnnNetwork::createNNet()
{
    net << conv(40, 40, 5, 40, 3, 10)      << activation::leaky_relu() // 40x5 kernel, 3 channel, 10 filter
        << ave_pool(36, 1, 10, 2, 1, 2, 1) << activation::leaky_relu() // pool 2x1, stride 2x1
        << conv(18, 1, 3, 1, 10, 20)       << activation::leaky_relu()
        << ave_pool(16, 1, 20, 2, 1, 2, 1) << activation::leaky_relu()
        << conv(8, 1, 8, 1, 20, 60)        << activation::leaky_relu() // flatten conv
        << fc(60, 2)                       << activation::softmax();
}

void EnnNetwork::train(float l_rate)
{
    bool need_train = load();
    if( !need_train )
    {
        benchmark();
        return;
    }

    qDebug() << "dataset size: test"  << dataset->test_images.size()
             << "train" << dataset->train_images.size();

    optim.alpha = l_rate; // learning rate = 1E-4
    nn_epoch = 0;

    net.fit<mse>(optim, dataset->train_images, dataset->train_labels,
                   n_minibatch, n_train_epochs, [&](){minibatchLog();},
                   [&](){epochLog();});

    if( is_wrong==0 )
    {
        save();
    }
    benchmark();
}

vec_t EnnNetwork::test(vec_t *data)
{
    vec_t res = net.predict(*data);
    return res;
}

void EnnNetwork::minibatchLog()
{

}

void EnnNetwork::epochLog()
{
    nn_epoch++;
    if( nn_epoch%ENN_EPOCH_LOG==0 )
    {
        qDebug() << "epoch" << nn_epoch << nn_t.elapsed() << "s elapsed.";
        float loss = calcLoss();
        result res_test = net.test(dataset->test_images, dataset->test_labels);
        result res = net.test(dataset->train_images, dataset->train_labels);

        qDebug() << res.num_success << "/" << res.num_total << "test"
                 << res_test.num_success << "/" << res_test.num_total << "loss:"
                 << loss << "alpha" << optim.alpha*1000;

        if( loss<ENN_TARGET_LOSS )
        {
            net.stop_ongoing_training();
        }

        double alpha = optim.alpha * 0.95;
        if( alpha>ENN_MIN_LRATE )
        {
            optim.alpha = alpha;
        }
        nn_t.restart();
    }
}

// return loss
float EnnNetwork::calcLoss()
{
    // calc loss
    float loss = 0;

    std::vector<tensor_t> label_tensor;
    net.normalize_tensor(dataset->train_labels, label_tensor);
    int len = dataset->train_images.size();

    float        wrong_loss = 0;
    QVector<int> wrong_i;
    for( int i=0 ; i<len ; i++ )
    {
        vec_t predicted = net.predict(dataset->train_images[i]);
        float s_loss = mse::f(predicted, label_tensor[i][0]);

        if( s_loss>0.95 )
        {
            wrong_loss += s_loss;
            wrong_i.push_back(i);
        }
        loss += s_loss;
    }
    if( wrong_loss>0 )
    {
        float diff = loss - wrong_loss;
        if( diff<ENN_TARGET_LOSS && wrong_i.length()<5 )
        {
            for( int i=0 ; i<wrong_i.length() ; i++ )
            {
                QString path = dataset->train_path[wrong_i[i]];
                if( path.contains(dataset->m_name) )
                {
                    qDebug() << "XxX wrong" << wrong_i[i] << path
                             << wrong_loss << "XxX";
                }
                else
                {
                    qDebug() << ">>> have 2 word?" << path
                             << wrong_loss << "<<<";
                }
            }
            net.stop_ongoing_training();
            is_wrong = 1;
        }
    }
    if( loss>90 )
    {
        qDebug() << "========= NO CONVERGANCE"
                 << dataset->m_name
                 << " ==========";
        net.stop_ongoing_training();
        is_wrong = 2;
    }

    return loss;
}
