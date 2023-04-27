#include "bt_network.h"
#include <QImage>
#include <qmath.h>

using namespace tiny_dnn;

BtNetwork::BtNetwork(BtState *state)
{
    word_list = bt_parseLexicon(state->word_path);

    int len = word_list.size();
    nets.resize(len);
    for( int i=0 ; i<len ; i++ )
    {
        QRegExp rxp("[<!#]");
        if( word_list[i].contains(rxp) )
        {
            continue; // skip silence and unknown
        }
        nets[i] = new TdNetwork(word_list[i]);
    }
}

BtNetwork::~BtNetwork()
{
    int len = nets.size();
    for( int i=0 ; i<len ; i++ )
    {
        delete nets[i];
    }
}

float BtNetwork::predict(int id)
{
#ifdef ENN_IMAGE_DATASET
    int data_len = BT_ENN_SIZE*BT_ENN_SIZE*3;
#else
    int data_len = BT_ENN_SIZE*BT_ENN_SIZE;
#endif
    vec_t res = nets[id]->predict(data_buf, data_len);
//    qDebug() << word_list[id]
//                << "Detect:" << res[0] << res[1];

    return res[1];
}

float BtNetwork::getConf(int start, int len, int id)
{
    if( !nets[id]->model_loaded )
    {
        return 0;
    }
//    qDebug() << "ww" << word_list[id] << nets[id]->m_name;

#ifdef ENN_IMAGE_DATASET
    fillImage(start, len);
#else
    fillRaw(start, len);
#endif

    return predict(id);
}

void BtNetwork::fillRaw(int start, int len)
{
    double scale_val = (len-1.0)/(BT_ENN_SIZE-1.0);
    int width  = BT_ENN_SIZE;
    int height = BT_ENN_SIZE;

    BtFrameBuf *buf_l; //left
    BtFrameBuf *buf_r; //right

    for( int i=0 ; i<height ; i++ )
    {
        for( int j=0 ; j<width ; j++ )
        {
            double p = j*scale_val;
            int p_round = qRound(p);
            if( qAbs(p-p_round)<0.001 )
            {
                // if remove this line calc error would
                // cause segfault in p_left or p_right
                // in the marginal values
                p = p_round;
            }
            int p_right = qCeil(p);
            int p_left  = qFloor(p);
            double ds = enn_getDimScale(p, len);

            double d = p-p_left;

            buf_l = cfb->get(start + p_left);
            buf_r = cfb->get(start + p_right);

            float val = d   *buf_r->enn[i]+
                       (1-d)*buf_l->enn[i];

            data_buf[i*width+j] = val * ds;
        }
    }
}

void BtNetwork::fillImage(int start, int len)
{
    calcStat(start, len);

    QImage *img = new QImage(len, BT_ENN_SIZE, QImage::Format_RGB888);
    for( int i=0 ; i<len ; i++ )
    {
        BtFrameBuf *buf = cfb->get(start + i);
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            double val = buf->enn[j];
            val -= offset_delta;
            val /= scale_delta;
            val *= 0.15;
            val += 0.5;
            if( val>1 )
            {
                val = 1;
            }
            if( val<0 )
            {
                val = 0;
            }

            float sat_col = 1;
            float hue_col = (1 - val) * 256/360.0;
            float val_col = val;
            QColor pixel;
            pixel.setHsvF(hue_col, sat_col, val_col);
            img->setPixelColor(i, j, pixel);
        }
    }

    img_s = img->scaled(BT_ENN_SIZE, BT_ENN_SIZE);
    int height = BT_ENN_SIZE;
    int width  = BT_ENN_SIZE;
    int ch_off = height * width;

    for( int i=0 ; i<height ; i++ )
    {
        for( int j=0 ; j<width ; j++ )
        {
            QRgb color = img_s.pixel(j, i);
            data_buf[i*width+j+0*ch_off] = qRed(color);
            data_buf[i*width+j+1*ch_off] = qGreen(color);
            data_buf[i*width+j+2*ch_off] = qBlue(color);
        }
    }
}

void BtNetwork::calcStat(int start, int len)
{
    int N = len * BT_ENN_SIZE;

    double var = 0;
    double sum = 0;

    for( int i=0 ; i<len ; i++ )
    {
        BtFrameBuf *buf = cfb->get(start + i);
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            double val = buf->enn[j];
            sum += val;
            var += qPow(val, 2);
        }
    }
    double mean = sum/N;
    var = qSqrt(var/N - qPow(mean, 2));
    offset_delta = mean;
    scale_delta = var;
}

void BtNetwork::saveWave(int start, int len, QString word)
{
//    double end = o_decoder->uframe/100.0;
//    double rw_len = end - (start + len); // rewind length
//    double word_len;

//    QString fname = KAL_AU_DIR"tt/";
//    mkDir(path);
//    fname = word + ".wav";

//    rw_len *= BT_REC_RATE/1000.0;
//    cy_buf->rewind(rw_len);
//    word_len = len * BT_REC_RATE/1000.0;

//    wav_w->writeEnn(path, word_len);
}
