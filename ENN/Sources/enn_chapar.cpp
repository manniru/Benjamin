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
    *disp += n_minibatch;
}

void EnnChapar::epochLog()
{
    result res = net.test(test_images, test_labels);
    qDebug() << res.num_success << "/" << res.num_total;
    qDebug() << (("epoch_"+to_string(nn_epoch++)).c_str()) << nn_t.elapsed() << "s elapsed.";
    disp->restart(train_images.size());
    nn_t.restart();
}

void EnnChapar::createEnn(QString word)
{
    // add layers
    // add layers
    core::backend_t backend_type = core::default_engine();
    net << conv(32, 32, 5, 1, 6,   // C1, 1@32x32-in, 6@28x28-out
                 padding::valid, true, 1, 1, 1, 1, backend_type)
         << activation::tanh()
         << ave_pool(28, 28, 6, 2)   // S2, 6@28x28-in, 6@14x14-out
         << activation::tanh()
         << conv(14, 14, 5, 6, 16,   // C3, 6@14x14-in, 16@10x10-out
                 core::connection_table(tbl, 6, 16),
                 padding::valid, true, 1, 1, 1, 1, backend_type)
         << activation::tanh()
         << ave_pool(10, 10, 16, 2)  // S4, 16@10x10-in, 16@5x5-out
         << activation::tanh()
         << conv(5, 5, 5, 16, 120,   // C5, 16@5x5-in, 120@1x1-out
                 padding::valid, true, 1, 1, 1, 1, backend_type)
         << activation::tanh()
         << fc(120, 10, true, backend_type)  // F6, 120-in, 10-out
         << activation::tanh();

    assert(net.in_data_size() == 32 * 32);
    assert(net.out_data_size() == 10);

    // load MNIST dataset


    parse_mnist_labels("train-labels.idx1-ubyte", &train_labels);
    parse_mnist_images("train-images.idx3-ubyte", &train_images, -1.0, 1.0, 2, 2);

    parse_mnist_labels("t10k-labels.idx1-ubyte", &test_labels);
    parse_mnist_images("t10k-images.idx3-ubyte", &test_images, -1.0, 1.0, 2, 2);

//    assert(net.in_data_size() == 40 * 40);
//    assert(net.out_data_size() == 2);

//    parseImagesT(ENN_TRAIN_DIR, word);
//    parseImagesF(ENN_TRAIN_DIR, word);

    qDebug() << "test" << test_images.size()
             << "train" << train_images.size();

    // declare optimization algorithm
    adagrad optimizer;
    optimizer.alpha *= 4; // learning rate = 1

    n_minibatch = 16;
    n_train_epochs = 50;
    disp = new progress_display(train_images.size());
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
            vec_t label = {1,0};
//            train_labels.push_back(label);
        }
        else
        {
            test_images.push_back(vec);
            vec_t label = {1,0};
//            test_labels.push_back(label);
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
        int train_size = false_paths.size()*1;//0.9;

        for( int j=0 ; j<false_paths.size() ; j++ )
        {
            QString img_address = path + false_dirs[i] + "/" + false_paths[j];
            image<> rgb_img(img_address.toStdString().c_str(),
                            image_type::rgb);
            vec_t vec = rgb_img.to_vec();

            if( j<train_size )
            {
                train_images.push_back(vec);
                vec_t label = {0,1};
//                train_labels.push_back(label);
            }
            else
            {
                test_images.push_back(vec);
                vec_t label = {0,1};
//                test_la   bels.push_back(label);
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
