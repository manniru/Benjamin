#include "enn_dataset.h"
#include <QDataStream>

#ifdef ENN_IMAGE_DATASET

EnnDataset::EnnDataset(QString word, int test)
{
    m_name = word;

    if( test )
    {
        testFile(ENN_TRAIN_DIR);
    }
    else
    {
        parseImagesT(ENN_TRAIN_DIR);
        parseImagesF(ENN_TRAIN_DIR);
    }

    shuffleData();
}

EnnDataset::~EnnDataset()
{
}

void EnnDataset::parseImagesT(QString path)
{
    QString path_m_name = path + m_name + "/";
    QStringList image_filenames = enn_listImages(path_m_name);
    int len = image_filenames.size();
//    int len = 10;
    train_size = len*0.9;

    for( int i=0 ; i<len ; i++ )
    {
        QString img_address = path_m_name + image_filenames[i];
        addImagesT(img_address, i);
    }
}

void EnnDataset::parseImagesF(QString path)
{
    QStringList false_dirs = enn_listDirs(path + "/");

    //remove true word dir
    int t_index = false_dirs.indexOf(m_name);
    false_dirs.removeAt(t_index);

    int fd_len = false_dirs.size();
//    int m_name_len = 1;

    for( int i=0 ; i<fd_len ; i++ )
    {
        QStringList image_filenames = enn_listImages(path + false_dirs[i]);
        int len = image_filenames.size();
//        int len = 10;

        for( int j=0 ; j<len ; j++ )
        {
            QString img_address = path + false_dirs[i] + "/" + image_filenames[j];
            addImagesF(img_address, i, j);
        }
    }
}

void EnnDataset::addImagesT(QString path, int i)
{
    image<> rgb_img(path.toStdString(), image_type::rgb);
    vec_t vec = rgb_img.to_vec();

    if( i<train_size )
    {
        train_datas.push_back(vec);
        train_labels.push_back(1);
        train_path.push_back(path); // for debug purposes
    }
    else
    {
        test_datas.push_back(vec);
        test_labels.push_back(1);
    }
}

void EnnDataset::addImagesF(QString path, int i, int j)
{
    if( j<ENN_FALSE_COUNT )
    {
        train_size = ENN_FALSE_COUNT*0.8;
        image<> rgb_img(path.toStdString(), image_type::rgb);
        vec_t vec = rgb_img.to_vec();
        if( j<train_size )
        {
            train_datas.push_back(vec);
            train_labels.push_back(0);
            train_path.push_back(path); // for debug purposes
        }
        else if( i%6==0 )
        {
            test_datas.push_back(vec);
            test_labels.push_back(0);
        }
    }
    else if( j<20 )
    {
        image<> rgb_img(path.toStdString(), image_type::rgb);
        vec_t vec = rgb_img.to_vec();
        false_datas.push_back(vec);
    }
}

void EnnDataset::shuffleData()
{
    std::random_device r;
    std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};

    // create two random engines with the same state
    std::mt19937 eng1(seed);
    std::mt19937 eng2 = eng1;
    std::mt19937 eng3 = eng1;

    std::shuffle(train_labels.begin(), train_labels.end(), eng1);
    std::shuffle(train_datas.begin(), train_datas.end(), eng2);
    std::shuffle(train_path.begin()  , train_path.end()  , eng3);

    std::shuffle(test_labels.begin(), test_labels.end(), eng1);
    std::shuffle(test_datas.begin(), test_datas.end(), eng2);
}

// use for sanity test on std::shuffle
void EnnDataset::shuffleTest(std::mt19937 *eng1,
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

void EnnDataset::testFile(QString path)
{
    QString path_m_name = path + m_name + "/";
    QStringList data_filenames = enn_listImages(path_m_name);
    int len = data_filenames.size();

    qDebug() << "arch" << len << path_m_name;
    for( int i=0 ; i<len ; i++ )
    {
        QString data_path = path_m_name + data_filenames[i];
        image<> rgb_img(data_path.toStdString(), image_type::rgb);
        vec_t vec = rgb_img.to_vec();
    }
}

#endif // ENN_IMAGE_DATASET
