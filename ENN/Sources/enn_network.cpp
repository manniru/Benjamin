#include "enn_network.h"

using namespace tiny_dnn::activation;
using namespace tiny_dnn::layers;
//#define ENN_MNIST

#ifdef ENN_MNIST
#define ENN_EPOCH_LOG 1
#else
#define ENN_EPOCH_LOG 10
#endif

timer nn_t;
int nn_epoch = 0;

EnnNetwork::EnnNetwork(QString word)
{
    dataset = new EnnDataset(word);
}

EnnNetwork::~EnnNetwork()
{
    delete dataset;
#ifdef ENN_MNIST
    delete disp;
#endif
}

void EnnNetwork::minibatchLog()
{
#ifdef ENN_MNIST
    *disp += n_minibatch;
#endif
}

void EnnNetwork::shuffleTest(std::mt19937 *eng1,
                            std::mt19937 *eng2)
{
    std::vector<int> v1{1,2,3,4,5};
    std::vector<int> v2{1,2,3,4,5};
    std::vector<int> v3{11,12,13,14,15};
    std::vector<int> v4{21,22,23,24,25};

    std::shuffle(v1.begin(), v1.end(), *eng1);
    std::shuffle(v2.begin(), v2.end(), *eng2);

    std::shuffle(v3.begin(), v3.end(), *eng1);
    std::shuffle(v4.begin(), v4.end(), *eng2);
}

void EnnNetwork::epochLog()
{
    nn_epoch++;
    if( nn_epoch%ENN_EPOCH_LOG==0 )
    {
        qDebug() << (("epoch_"+to_string(nn_epoch)).c_str()) << nn_t.elapsed() << "s elapsed.";
        result res_test = net.test(dataset->test_images, dataset->test_labels);
#ifndef ENN_MNIST
        result res = net.test(dataset->train_images, dataset->train_labels);

//        std::vector<vec_t> vec;
//        net.net_.label2vec(train_labels, vec);

        qDebug() << res.num_success << "/" << res.num_total << "test"
                 << res_test.num_success << "/" << res_test.num_total << "loss:"
                 << net.get_loss<mse>(dataset->train_images, dataset->train_labels);
//                 << net.get_loss<mse>(train_images, vec);
#else
        qDebug() << res_test.num_success << "/" << res_test.num_total << "loss:";
//                 << net.get_loss<mse>(train_images, train_labels);
        disp->restart(train_images.size());
#endif
        nn_t.restart();
    }
}

void EnnNetwork::createEnn()
{
#ifndef ENN_MNIST
    // add layers
    net << conv(40, 40, 5, 40, 3, 10)      << activation::leaky_relu() // 40x5 kernel, 3 channel, 10 filter
        << ave_pool(36, 1, 10, 2, 1, 2, 1) << activation::leaky_relu() // pool 2x1, stride 2x1
        << conv(18, 1, 3, 1, 10, 20)       << activation::leaky_relu()
        << ave_pool(16, 1, 20, 2, 1, 2, 1) << activation::leaky_relu()
        << conv(8, 1, 8, 1, 20, 60)        << activation::leaky_relu() // flatten conv
        << fc(60, 2)                       << activation::softmax();

//    net << conv(40, 40, 5, 3, 10)   << activation::leaky_relu() // 40x5 kernel, 3 channel, 10 filter
//        << ave_pool(36, 36, 10, 2)  << activation::leaky_relu() // pool 2x1, stride 2x1
//        << conv(18, 18, 3, 10, 20)  << activation::leaky_relu()
//        << ave_pool(16, 16, 20, 2)  << activation::leaky_relu()
//        << conv(8, 8, 8, 8, 20, 60) << activation::leaky_relu() // flatten conv
//        << fc(60, 2)                << activation::softmax();

#else
    // add layers
    net << conv(32, 32, 5, 5, 1, 5)         << activation::relu() // 5x5 kernel, 3 channel
        << ave_pool(28, 28, 5, 2, 2, 2, 2)  << activation::relu() //pool 2x1, stride 2x1
        << conv(14, 14, 5, 5, 5, 10)        << activation::relu()
        << ave_pool(10, 10, 10, 2, 2, 2, 2) << activation::relu()
        << conv(5, 5, 5, 5, 10, 60)         << activation::relu()
        << fc(60, 10)                       << activation::sigmoid();

    // load MNIST dataset
    parse_mnist_labels("train-labels.idx1-ubyte", &train_labels);
    parse_mnist_images("train-images.idx3-ubyte", &train_images, 0, 255, 2, 2);

    parse_mnist_labels("t10k-labels.idx1-ubyte", &test_labels);
    parse_mnist_images("t10k-images.idx3-ubyte", &test_images, 0, 255, 2, 2);
#endif

    // declare optimization algorithm
    adagrad optimizer;
    optimizer.alpha *= 0.1; // learning rate = 1E-4

    n_minibatch = 16;
    n_train_epochs = 100;
#ifdef ENN_MNIST
    disp = new progress_display(train_images.size());
#endif
    net.fit<mse>(optimizer, dataset->train_images, dataset->train_labels,
                 n_minibatch, n_train_epochs, [&](){minibatchLog();},
                 [&](){epochLog();});

    // save
    net.save(dataset->m_name.toStdString().c_str());


    tiny_dnn::timer t;  // start the timer

    // predict
    vec_t res = net.predict(dataset->test_images[0]);

    double elapsed_s = t.elapsed();
    t.stop();
    qDebug() << "Finished!" << elapsed_s;
    qDebug() << "Detect:" << res[0] << res[1];

    exit(0);
    // load
    // network<sequential> net2;
    // net2.load("net");
}

