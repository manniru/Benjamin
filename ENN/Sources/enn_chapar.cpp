#include "enn_chapar.h"

using namespace tiny_dnn::activation;
using namespace tiny_dnn::layers;

EnnChapar::EnnChapar()
{

}

void EnnChapar::createEnn(QString word)
{
    network<sequential> net;

    // add layers
    net << conv(32, 32, 5, 1, 6)  << activation::tanh() // in:32x32x1, 5x5conv, 6fmaps
        << ave_pool(28, 28, 6, 2) << activation::tanh() // in:28x28x6, 2x2pooling
        << fc(14 * 14 * 6, 120)   << activation::tanh() // in:14x14x6, out:120
        << fc(120, 2);                                  // in:120,     out:10

    assert(net.in_data_size() == 32 * 32);
    assert(net.out_data_size() == 10);

    parseImages(ENN_TRAIN_DIR, word);

    // declare optimization algorithm
    adagrad optimizer;

    // train (50-epoch, 30-minibatch)
    net.train<mse, adagrad>(optimizer, train_images, train_labels, 30, 50);

    // save
    net.save(word.toStdString().c_str());

    // load
    // network<sequential> net2;
    // net2.load("net");
}

void EnnChapar::parseImages(QString path, QString word)
{
    vector<vec_t> train_images;

    QString path_word = path + "/" + word + "/";
    QStringList word_images = listImages(path_word);

    for( int i=0 ; i<word_images.size() ; i++ )
    {
        QString img_address = path_word + word_images[i];
        image<> rgb_img(img_address.toStdString().c_str(), tiny_dnn::image_type::rgb);
        vec_t vec = rgb_img.to_vec();

        train_images.push_back(vec);
        train_labels.push_back(1);
    }

    QStringList false_dirs = listDirs(path + "/");
    int word_dir_index = false_dirs.indexOf(word);
    false_dirs.removeAt(word_dir_index);

    for( int i=0 ; i<false_dirs.size() ; i++ )
    {
        QStringList false_paths = listImages(path + false_dirs[i], ENN_FALSE_COUNT);

        for( int j=0 ; j<false_paths.size() ; j++ )
        {
            QString img_address = path + false_dirs[i] + word_images[j];
            image<> rgb_img(img_address.toStdString().c_str(), image_type::rgb);
            vec_t vec = rgb_img.to_vec();

            train_images.push_back(vec);
            train_labels.push_back(0);
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
