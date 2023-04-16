#include "enn_test.h"

// read one true sample from "word" data set
EnnTest::EnnTest(QString word)
{
    data_dir = ENN_TRAIN_DIR + word + "/";
    readFirstSample();
}

EnnTest::~EnnTest()
{
}

void EnnTest::readFirstSample()
{
    QStringList filenames = listImages(data_dir);
    img_address = data_dir + filenames[0];

#ifdef ENN_IMAGE_DATASET
    image<> rgb_img(img_address.toStdString().c_str(),
                    tiny_dnn::image_type::rgb);
    image = rgb_img.to_vec();
#else
    enn_readENN(img_address, &image);
#endif
}

QStringList EnnTest::listImages(QString path, int num)
{
    QDir p_dir(path);
    QStringList fmt;
#ifdef ENN_IMAGE_DATASET
    fmt.append("*.png");
#else
    fmt.append("*.enn");
#endif
    QStringList file_list = p_dir.entryList(fmt, QDir::Files);

    if( num<file_list.size() && num>0 )
    {
        file_list = file_list.mid(0, num);
    }
    return file_list;
}
