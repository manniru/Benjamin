#include "enn_chapar.h"

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

EnnChapar::EnnChapar()
{

}

void EnnChapar::minibatchLog()
{
#ifdef ENN_MNIST
    *disp += n_minibatch;
#endif
}

void EnnChapar::shuffleTest(std::mt19937 *eng1,
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

void EnnChapar::epochLog()
{
    nn_epoch++;
    if( nn_epoch%ENN_EPOCH_LOG==0 )
    {
        qDebug() << (("epoch_"+to_string(nn_epoch)).c_str()) << nn_t.elapsed() << "s elapsed.";
        result res_test = net.test(test_images, test_labels);
#ifndef ENN_MNIST
        result res = net.test(train_images, train_labels);

//        std::vector<vec_t> vec;
//        net.net_.label2vec(train_labels, vec);

        qDebug() << res.num_success << "/" << res.num_total << "test"
                 << res_test.num_success << "/" << res_test.num_total << "loss:"
                 << net.get_loss<mse>(train_images, train_labels);
//                 << net.get_loss<mse>(train_images, vec);
#else
        qDebug() << res_test.num_success << "/" << res_test.num_total << "loss:";
//                 << net.get_loss<mse>(train_images, train_labels);
        disp->restart(train_images.size());
#endif
        nn_t.restart();
    }
}

void EnnChapar::createEnn(QString word)
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


    parseImagesT(ENN_TRAIN_DIR, word);
    parseImagesF(ENN_TRAIN_DIR, word);
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

    std::random_device r;
    std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};

    // create two random engines with the same state
    std::mt19937 eng1(seed);
    std::mt19937 eng2 = eng1;

    std::shuffle(begin(train_labels), end(train_labels), eng1);
    std::shuffle(begin(train_images), end(train_images), eng2);

    std::shuffle(begin(test_labels), end(test_labels), eng1);
    std::shuffle(begin(test_images), end(test_images), eng2);

    qDebug() << "test"  << test_images.size()
             << "train" << train_images.size();

    // declare optimization algorithm
    adagrad optimizer;
    optimizer.alpha *= 0.1; // learning rate = 1E-4

    n_minibatch = 16;
    n_train_epochs = 40;
#ifdef ENN_MNIST
    disp = new progress_display(train_images.size());
#endif
    net.fit<mse>(optimizer, train_images, train_labels, n_minibatch,
                 n_train_epochs, [&](){minibatchLog();}, [&](){epochLog();});

    // save
    net.save(word.toStdString().c_str());


    tiny_dnn::timer t;  // start the timer

    // predict
    vec_t res = net.predict(test_images[0]);

    double elapsed_s = t.elapsed();
    t.stop();
    qDebug() << "Finished!" << elapsed_s;

    exit(0);
    // load
    // network<sequential> net2;
    // net2.load("net");
}

void EnnChapar::parseImagesT(QString path, QString word)
{
    QString path_word = path + word + "/";
    QStringList image_filenames = listImages(path_word);
    int len = image_filenames.size();
//    int len = 10;
    int train_size = len*0.9;

    for( int i=0 ; i<len ; i++ )
    {
        QString img_address = path_word + image_filenames[i];
        image<> rgb_img(img_address.toStdString().c_str(),
                        tiny_dnn::image_type::rgb);
        vec_t vec = rgb_img.to_vec();
        vec_t label = {1,0};

        if( i<train_size )
        {
            train_images.push_back(vec);
            train_labels.push_back(1);
            train_labels_v.push_back(label);
        }
        else
        {
            test_images.push_back(vec);
            test_labels.push_back(1);
            test_labels_v.push_back(label);
        }
    }
}

void EnnChapar::parseImagesF(QString path, QString word)
{
    QStringList false_dirs = listDirs(path + "/");
    int word_dir_index = false_dirs.indexOf(word);
    false_dirs.removeAt(word_dir_index);

//    for( int i=0 ; i<false_dirs.size() ; i++ )
    for( int i=0 ; i<1 ; i++ )
    {
        QStringList image_filenames = listImages(path + false_dirs[i]);
//        QStringList false_paths = listImages(path + false_dirs[i],
//                                             ENN_FALSE_COUNT);
        int len = image_filenames.size();
//        int len = 10;
        int train_size = len*0.9;

        for( int j=0 ; j<len ; j++ )
        {
            QString img_address = path + false_dirs[i] + "/" + image_filenames[j];
            image<> rgb_img(img_address.toStdString().c_str(),
                            image_type::rgb);
            vec_t vec = rgb_img.to_vec();
            vec_t label = {0,1};

            if( j<train_size )
            {
                train_images.push_back(vec);
                train_labels.push_back(0);
                train_labels_v.push_back(label);
            }
            else
            {
                test_images.push_back(vec);
                test_labels.push_back(0);
                test_labels_v.push_back(label);
            }
        }
    }
}

QStringList EnnChapar::listImages(QString path, int num)
{
    QDir p_dir(path);
    QStringList fmt;
    fmt.append("*.png");
    QStringList file_list = p_dir.entryList(fmt, QDir::Files);

    if( num<file_list.size() && num>0 )
    {
        file_list = file_list.mid(0, num);
    }
    return file_list;
}

QStringList EnnChapar::listDirs(QString path)
{
    QDir p_dir(path);
    QStringList fmt;
    fmt.append("*");
    QStringList dir_list = p_dir.entryList(fmt, QDir::Dirs | QDir::NoDotAndDotDot);

    return dir_list;
}

EnnChapar::~EnnChapar()
{
}
