#include "enn_test.h"

EnnTest::EnnTest(QString word)
{
    m_name = word;

    parseImages(ENN_TRAIN_DIR);
}

EnnTest::~EnnTest()
{
}

void EnnTest::parseImages(QString path)
{
    QString path_m_name = path + m_name + "/";
    QStringList image_filenames = listImages(path_m_name);

    img_address = path_m_name + image_filenames[0];
//    QImage im_buf(img_address);
//    images.resize(BT_ENN_SIZE*BT_ENN_SIZE*3);

//    int height = im_buf.height();
//    int width = im_buf.width();
//    int ch_off = height * width;

//    for( int i=0 ; i<height ; i++ )
//    {
//        for( int j=0 ; j<width ; j++ )
//        {
//            QRgb color = im_buf.pixel(i, j);
//            images[i*width+j+0*ch_off] = qRed(color);
//            images[i*width+j+1*ch_off] = qGreen(color);
//            images[i*width+j+2*ch_off] = qBlue(color);
//        }
//    }


    image<> rgb_img(img_address.toStdString().c_str(),
                    tiny_dnn::image_type::rgb);
    images = rgb_img.to_vec();
}

QStringList EnnTest::listImages(QString path, int num)
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
