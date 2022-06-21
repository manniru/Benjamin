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

    setbuf(stdout,NULL); //to work out printf
}

EnnNetwork::~EnnNetwork()
{
    delete dataset;
}

void EnnNetwork::benchmark()
{
    tiny_dnn::timer t;  // start the timer

    // predict
    net.predict(dataset->test_images[0]);

    double elapsed_s = t.elapsed();
    t.stop();
    qDebug() << dataset->m_name << "Finished!"
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
        qDebug() << "model " << model_path << "loaded"
                 << "loss"   << loss ;
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
        return;
    }

    qDebug() << "dataset size: test"  << dataset->test_images.size()
             << "train" << dataset->train_images.size();

    optim.alpha = l_rate; // learning rate = 1E-4
    nn_epoch = 0;

    net.fit<mse>(optim, dataset->train_images, dataset->train_labels,
                   n_minibatch, n_train_epochs, [&](){;},
                   [&](){epochLog();});

    if( is_wrong!=2 )
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

void EnnNetwork::epochLog()
{
    nn_epoch++;
    if( nn_epoch%ENN_EPOCH_LOG==0 )
    {
        QString t_elapsed = QString::number(nn_t.elapsed());
        t_elapsed += "s";
        float loss = calcLoss();
        QString acc_test = getAcc(dataset->test_images,
                                  dataset->test_labels);
        QString acc_train = getAcc(dataset->train_images,
                                  dataset->train_labels);

        qDebug() << nn_epoch << t_elapsed
                 << acc_train << "test" << acc_test << "loss:"
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

    float          wrong_sum;
    QVector<int>   wrong_i;
    QVector<float> wrong_loss;
    for( int i=0 ; i<len ; i++ )
    {
        vec_t predicted = net.predict(dataset->train_images[i]);
        float s_loss = mse::f(predicted, label_tensor[i][0]);

        if( s_loss>0.95 )
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
        handleWrongs(diff, wrong_i, wrong_loss);
    }
    if( loss>200 )
    {
        qDebug() << "========= NO CONVERGANCE"
                 << dataset->m_name
                 << " ==========";
        net.stop_ongoing_training();
        is_wrong = 2;
    }

    return loss;
}

QString EnnNetwork::getAcc(std::vector<vec_t>   &data,
                           std::vector<label_t> &label)
{
    QString ret = "T[";
    int tot_t = 0; //total true
    int det_t = 0; //corrrect detected true
    int tot_f = 0; //total false
    int det_f = 0; //corrrect detected false

    int len = data.size();
    for( int i=0 ; i<len ; i++ )
    {
        vec_t out = test(&(data[i]));
        if( label[i]==1 )
        {
            tot_t++;
            if( out[1]>0.6 )
            {
                det_t++;
            }
        }
        else
        {
            tot_f++;
            if( out[0]>0.6 )
            {
                det_f++;
            }
        }
    }

    ret += QString::number(det_t) + "/";
    ret += QString::number(tot_t) + "] F[";

    ret += QString::number(det_f) + "/";
    ret += QString::number(tot_f) + "] Tot[";

    ret += QString::number(det_t+det_f) + "/";
    ret += QString::number(len) + "]";

    return ret;
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
        net.stop_ongoing_training();
        is_wrong = 1;

        QString acc_test = getAcc(dataset->test_images,
                                  dataset->test_labels);
        QString acc_train = getAcc(dataset->train_images,
                                  dataset->train_labels);

        qDebug() << "train" << acc_train
                 << "test" << acc_test;
    }
}
