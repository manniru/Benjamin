#include "enn_sample_link.h"
#include <QDir>
#include <QtMath>
#include <tiny_dnn/util/util.h>

EnnSampleLink::EnnSampleLink(QObject *parent) :
    QObject(parent), QQuickImageProvider(QQuickImageProvider::Pixmap)
{
    enn_path = ab_getAudioPath() + "enn";
    enn_path += QDir::separator();
}

QPixmap EnnSampleLink::requestPixmap(const QString &id, QSize *size,
                                     const QSize &requestedSize)
{
    int width = BT_ENN_SIZE;
    int height = BT_ENN_SIZE;
    QStringList id_split = id.split("/");
    QString word = id_split[0];
    QString name = id_split[1];
    QPixmap pixmap(width, height);

    if( size )
    {
        *size = QSize(width, height);
    }

    if( name.isEmpty() )
    {
        qDebug() << "file name is empty";
        return pixmap;
    }

    QString file_path = enn_path + id;
    file_path.replace("/", "\\");
//    qDebug() << "requestPixmap" << file_path;
    if( !QFile::exists(file_path) )
    {
        qDebug() << "file not exist";
        return pixmap;
    }

    tiny_dnn::vec_t vec;
    enn_readENN(file_path, &vec);

    QImage image(width, height, QImage::Format_RGB888);
    calcStat(vec);
    for( int i=0 ; i<height ; i++ )
    {
        for( int j=0 ; j<width ; j++ )
        {
            double val = vec[i*height+j]*10;
            val -= offset_delta;
            val -= 40;
            val /= scale_delta;
            val *= 0.15;
            val += 0.5;
            val /= 7;
//            qDebug() << i << j << ")"
//                     << val;
            if( val>1 )
            {
                val = 1;
            }
            if( val<0 )
            {
                val = 0;
            }

            float hue_col = (1 - val) * 256/360.0;
            float sat_col = 1;
            float val_col = val;
            QColor color;
            color.setHsvF(hue_col, sat_col, val_col);
            image.setPixelColor(j, i, color);

        }
    }
    pixmap = QPixmap::fromImage(image);
    return pixmap;
}

void EnnSampleLink::calcStat(tiny_dnn::vec_t data)
{
    double var = 0;
    int N = BT_ENN_SIZE * BT_ENN_SIZE;

    double sum = 0;

    for( int i=0 ; i<BT_ENN_SIZE ; i++ )
    {
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            double val = data[i*BT_ENN_SIZE+j];
            sum += val;
            var += val * val;
        }
    }
    double mean = sum/N;
    var = qSqrt(var/N - qPow(mean, 2));
    offset_delta = mean;
    scale_delta = var;
}
