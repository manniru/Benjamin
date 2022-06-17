#include "enn_dataset.h"

EnnDataset::EnnDataset(QString word)
{
    m_name = word;

    parseImagesT(ENN_TRAIN_DIR);
    parseImagesF(ENN_TRAIN_DIR);

    shuffleData();

    qDebug() << "dataset size: test"  << test_images.size()
             << "train" << train_images.size();
}

void EnnDataset::parseImagesT(QString path)
{
    QString path_m_name = path + m_name + "/";
    QStringList image_filenames = listImages(path_m_name);
    int len = image_filenames.size();
//    int len = 10;
    int train_size = len*0.9;

    for( int i=0 ; i<len ; i++ )
    {
        QString img_address = path_m_name + image_filenames[i];
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

void EnnDataset::parseImagesF(QString path)
{
    QStringList false_dirs = listDirs(path + "/");
    int m_name_dir_index = false_dirs.indexOf(m_name);
    false_dirs.removeAt(m_name_dir_index);
    int m_name_len = false_dirs.size();
//    int m_name_len = 1;

    for( int i=0 ; i<m_name_len ; i++ )
    {
        QStringList image_filenames = listImages(path + false_dirs[i],
                                             ENN_FALSE_COUNT);
        int len = image_filenames.size();
//        QStringList image_filenames = listImages(path + false_dirs[i]);
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
            else if( i%6==0 )
            {
                test_images.push_back(vec);
                test_labels.push_back(0);
                test_labels_v.push_back(label);
            }
        }
    }
}

void EnnDataset::shuffleData()
{
    std::random_device r;
    std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};

    // create two random engines with the same state
    std::mt19937 eng1(seed);
    std::mt19937 eng2 = eng1;

    std::shuffle(train_labels.begin(), train_labels.end(), eng1);
    std::shuffle(train_images.begin(), train_images.end(), eng2);

    std::shuffle(test_labels.begin(), test_labels.end(), eng1);
    std::shuffle(test_images.begin(), test_images.end(), eng2);
}

QStringList EnnDataset::listImages(QString path, int num)
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

QStringList EnnDataset::listDirs(QString path)
{
    QDir p_dir(path);
    QStringList fmt;
    fmt.append("*");
    QStringList dir_list = p_dir.entryList(fmt, QDir::Dirs | QDir::NoDotAndDotDot);

    return dir_list;
}

EnnDataset::~EnnDataset()
{
}
