#include "enn_chapar.h"

using namespace tiny_dnn::activation;
using namespace tiny_dnn::layers;

timer nn_t;
int nn_epoch = 0;

EnnChapar::EnnChapar()
{

}

void minibatchLog()
{
    nn_t.elapsed();
    nn_t.restart();
}

void EnnChapar::epochLog()
{
    result res = net.test(test_images, test_labels);
    cout << res.num_success << "/" << res.num_total << endl;
    cout << (("epoch_"+to_string(nn_epoch++)).c_str());
//    cout << net.get_loss<mse>(train_images, train_labels);
//    net.get_loss<tiny_dnn::mse>(X, sinusX);
//    cout << net;
}

void EnnChapar::createEnn(QString word)
{
    // add layers
    net << conv(40, 40, 5, 3, 6)  << activation::tanh() // in:40x40x1, 5x5conv, 6fmaps
        << ave_pool(36, 36, 6, 2) << activation::tanh() // in:36x36x6, 2x2pooling
        << fc(18 * 18 * 6, 120)   << activation::tanh() // in:18x18x6, out:120
        << fc(120, 2);                                  // in:120,     out:2

//    assert(net.in_data_size() == 40 * 40);
//    assert(net.out_data_size() == 2);

    parseImagesT(ENN_TRAIN_DIR, word);
    parseImagesF(ENN_TRAIN_DIR, word);

    qDebug() << "test" << test_images.size()
             << "train" << train_images.size();

    // declare optimization algorithm
    adagrad optimizer;

    // train (50-epoch, 30-minibatch)
//    net.train<mse, adagrad>(optimizer, train_images, train_labels, 30, 50);
    net.train<mse, adagrad>(optimizer, train_images, train_labels, 30, 10,
                                     minibatchLog, [&](){epochLog();});

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
            train_labels.push_back(1);
        }
        else
        {
            test_images.push_back(vec);
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
                train_labels.push_back(0);
            }
            else
            {
                test_images.push_back(vec);
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
