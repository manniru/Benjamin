#include "enn_dataset.h"

#define ENN_FALSE_COUNT_FT 20
#ifndef ENN_IMAGE_DATASET

EnnDataset::EnnDataset(QString word, int test)
{
    m_name = word;

    if( test )
    {
        testFile(ENN_TRAIN_DIR);
    }
    else
    {
        parseTrues(ENN_TRAIN_DIR);
        parseFalses(ENN_TRAIN_DIR);
    }

    shuffleData();
}

EnnDataset::~EnnDataset()
{
}

void EnnDataset::parseTrues(QString path)
{
    QString path_m_name = path + m_name + "/";
    QStringList filenames = enn_listDatas(path_m_name);
    int len = filenames.size();
//    int len = 10;
    train_size = len*0.9;

    for( int i=0 ; i<len ; i++ )
    {
        QString img_address = path_m_name + filenames[i];
        addDataT(img_address, i);
    }
}

void EnnDataset::parseFalses(QString path)
{
    QStringList false_dirs = enn_listDirs(path + "/");

    //remove true word dir
    int t_index = false_dirs.indexOf(m_name);
    false_dirs.removeAt(t_index);

    int fd_len = false_dirs.size();
//    int m_name_len = 1;

    for( int i=0 ; i<fd_len ; i++ )
    {
        QStringList filenames = enn_listDatas(path + false_dirs[i]);
        int len = filenames.size();
//        int len = 10;

        for( int j=0 ; j<len ; j++ )
        {
            QString img_address = path + false_dirs[i] + "/" + filenames[j];
            addDataF(img_address, i, j);
        }
    }
}

void EnnDataset::addDataT(QString path, int i)
{
    vec_t vec;
    enn_readENN(path, &vec);

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

void EnnDataset::addDataF(QString path, int i, int j)
{
    vec_t vec;
    if( j<ENN_FALSE_COUNT )
    {
        train_size = ENN_FALSE_COUNT*0.8;
        enn_readENN(path, &vec);
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
    else if( j<ENN_FALSE_COUNT_FT )
    {
        // for testFullMode
        enn_readENN(path, &vec);
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

void EnnDataset::testFile(QString path)
{
    QString path_m_name = path + m_name + "/";
    QStringList data_filenames = enn_listDatas(path_m_name);
    int len = data_filenames.size();

//    qDebug() << "arch" << len << path_m_name;
    for( int i=0 ; i<len ; i++ )
    {
        QString data_path = path_m_name + data_filenames[i];
        vec_t vec;
        enn_readENN(data_path, &vec);
    }
}

#endif // ENN_IMAGE_DATASET
