#include "enn_chapar.h"

using namespace tiny_dnn::activation;
using namespace tiny_dnn::layers;

timer nn_t;
int nn_epoch = 0;

EnnChapar::EnnChapar()
{

}

void EnnChapar::minibatchLog()
{
//    *disp += n_minibatch;
}

void EnnChapar::epochLog()
{
    nn_epoch++;
    if( nn_epoch%50==0 )
    {
        result res = net.test(train_images, train_labels);
        result res_test = net.test(test_images, test_labels);
        qDebug() << res.num_success << "/" << res.num_total << "test"
                 << res_test.num_success << "/" << res_test.num_total;
        qDebug() << (("epoch_"+to_string(nn_epoch)).c_str()) << nn_t.elapsed() << "s elapsed.";
//        disp->restart(train_images.size());
        nn_t.restart();
    }
}

void EnnChapar::createEnn(QString word)
{
    // add layers
    net << conv(40, 40, 5, 3, 6)   << activation::tanh()
        << ave_pool(36, 36, 6, 2)  << activation::tanh()
        << conv(18, 18, 5, 6, 16)  << activation::tanh()
        << ave_pool(14, 14, 16, 2) << activation::tanh()
        << conv(7, 7, 5, 16, 120)  << activation::tanh()
        << fc(3*3*120, 60)        << activation::tanh()
        << fc(60, 2)              << activation::tanh();

    // load MNIST dataset
//    parse_mnist_labels("train-labels.idx1-ubyte", &train_labels);
//    parse_mnist_images("train-images.idx3-ubyte", &train_images, 0, 255, 2, 2);

//    parse_mnist_labels("t10k-labels.idx1-ubyte", &test_labels);
//    parse_mnist_images("t10k-images.idx3-ubyte", &test_images, 0, 255, 2, 2);

    std::random_device r;
    std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};

    // create two random engines with the same state
    std::mt19937 eng1(seed);
    std::mt19937 eng2 = eng1;

    parseImagesT(ENN_TRAIN_DIR, word);
    parseImagesF(ENN_TRAIN_DIR, word);

    std::shuffle(begin(train_labels), end(train_labels), eng1);
    std::shuffle(begin(train_images), end(train_images), eng2);

    std::shuffle(begin(test_labels), end(test_labels), eng1);
    std::shuffle(begin(test_images), end(test_images), eng2);

    qDebug() << "test" << test_images.size()
             << "train" << train_images.size();

    // declare optimization algorithm
    adagrad optimizer;
    optimizer.alpha *= 4; // learning rate = 1

    n_minibatch = 16;
    n_train_epochs = 2000;
//    disp = new progress_display(train_images.size());
    net.fit<mse>(optimizer, train_images, train_labels, n_minibatch, n_train_epochs,
                 [&](){minibatchLog();}, [&](){epochLog();});

    // save
    net.save(word.toStdString().c_str());

    qDebug() << "Finished!";
    exit(0);
    // load
    // network<sequential> net2;
    // net2.load("net");
}

void EnnChapar::parseImagesT(QString path, QString word)
{
    QString path_word = path + word + "/";
    QStringList word_images = listImages(path_word);
    int train_size = word_images.size()*0.9;

    for( int i=0 ; i<word_images.size() ; i++ )
    {
        QString img_address = path_word + word_images[i];
        image<> rgb_img(img_address.toStdString().c_str(),
                        tiny_dnn::image_type::rgb);
        vec_t vec = rgb_img.to_vec();

        if( i<train_size )
        {
            train_images.push_back(vec);
//            vec_t label = {1,0};
//            train_labels.push_back(label);
            train_labels.push_back(1);
        }
        else
        {
            test_images.push_back(vec);
//            vec_t label = {1,0};
//            test_labels.push_back(label);
            test_labels.push_back(1);
        }
    }
}

void EnnChapar::parseImagesF(QString path, QString word)
{
    QStringList false_dirs = listDirs(path + "/");
    int word_dir_index = false_dirs.indexOf(word);
    false_dirs.removeAt(word_dir_index);

    for( int i=0 ; i<false_dirs.size() ; i++ )
    {
        QStringList false_paths = listImages(path + false_dirs[i],
                                             ENN_FALSE_COUNT);
        int train_size = false_paths.size()*0.9;

        for( int j=0 ; j<false_paths.size() ; j++ )
        {
            QString img_address = path + false_dirs[i] + "/" + false_paths[j];
            image<> rgb_img(img_address.toStdString().c_str(),
                            image_type::rgb);
            vec_t vec = rgb_img.to_vec();

            if( j<train_size )
            {
                train_images.push_back(vec);
//                vec_t label = {0,1};
//                train_labels.push_back(label);
                train_labels.push_back(0);
            }
            else
            {
                test_images.push_back(vec);
//                vec_t label = {0,1};
//                test_labels.push_back(label);
                test_labels.push_back(0);
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
