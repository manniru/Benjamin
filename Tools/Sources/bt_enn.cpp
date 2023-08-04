#include "bt_enn.h"
#include <QDir>
#include <qmath.h>

BtEnn::BtEnn(QString dir_name, BtState *state,
             QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);
    st     = state;

    QDir p_dir(dir_name);
    QStringList fmt;
    fmt.append("*.wav");
    file_list = p_dir.entryList(fmt, QDir::Files);

    for( int i = 0 ; i<file_list.size() ; i++ )
    {
        file_list[i] = dir_name + file_list[i];
    }

    for( int i=0 ; i<BT_DELTA_ORDER+1 ; i++)
    {
        max_delta[i] = 0;
        min_delta[i] = 10000;
    }

    enn_dir = ab_getAudioPath() + "enn";
    enn_dir += QDir::separator();
    bt_mkDir(enn_dir);
    shit_dir = ab_getAudioPath() + "shit";
    shit_dir += QDir::separator();
    bt_mkDir(shit_dir);

    wav_w = new BtWavWriter(cy_buf, st);

    fillExist();
}

BtEnn::~BtEnn()
{
    delete t_model;
    delete oa_model;
    delete o_decoder;
    delete cy_buf;
}

void BtEnn::init(QString dir)
{
    cat_dir = dir;
    std::string model_filename = st->mdl_path;

    oa_model = new KdAModel;
    t_model = new KdTransitionModel;

    std::ifstream ki;
    kd_openFile(model_filename, &ki);
    t_model->Read(ki);
    oa_model->Read(ki);

    o_decoder = new KdOnlineLDecoder(t_model, st);
    st->min_sil = 300;

    startDecode();
}

void BtEnn::startDecode()
{
    KdDecodable decodable(cy_buf, oa_model,
                          t_model, st);
    decodable.features->enableENN();

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;

    //    int len = 50;
    int len = file_list.size();
    qDebug() << "size" << file_list.size() << cat_dir;
    for( int i=0 ; i<len ; i++ )
    {
        if( checkExist(file_list[i]) )
        {
            continue;
        }
        qDebug() << "Info: wave [" << i << "/" << len << "].";
        openWave(file_list[i]);
        readWav(cy_buf);
        decodable.features->ComputeFeatures();
        o_decoder->Decode();
        last_r = o_decoder->getResult(&out_fst);
        if( !sanityCheck(file_list[i]) )
        {
            shit_counter++;
            qDebug("shit[%d] %s %d", shit_counter,
                   file_list[i].toStdString().c_str(), last_r.size());
            bt_printResult(last_r);
            o_decoder->wav_id++;
            o_decoder->resetODecoder();
            delete decodable.features->cmvn;
            decodable.features->cmvn = new BtCMVN(
                        decodable.features->o_features, st);
            decodable.features->delta->min_frame = o_decoder->status.min_frame;
            last_r.clear();
            if( shit_counter<BT_MAX_SHIT )
            {
                wav_file.close();
                moveToShit(file_list[i]);
                continue;
            }
            exit(1);
        }
        preProcess();
        saveFeature(file_list[i], decodable.features->o_features);
//        saveWave(file_list[i]);
        o_decoder->wav_id++;
        o_decoder->resetODecoder();
        delete decodable.features->cmvn;
        decodable.features->cmvn = new BtCMVN(
                    decodable.features->o_features, st);
        decodable.features->delta->min_frame = o_decoder->status.min_frame;
        last_r.clear();
    }
    qDebug() << "shit [" << shit_counter << "]";
}

void BtEnn::openWave(QString filename)
{
    if( wav_file.isOpen() )
    {
        wav_file.close();
    }

    wav_file.setFileName(filename);
    if( !wav_file.open(QIODevice::ReadOnly) )
    {
        qDebug() << "(openWave) Failed To Open" << filename;
        exit(1);
    }
    //    qDebug() << ">>>>" << QFileInfo(filename).fileName();

    char buff[200];

    wav_file.read(4); // ="RIFF" is the father of wav
    wav_file.read(buff,4);//chunk size(int)
    wav_file.read(buff,4);//format="WAVE"
    wav_file.read(buff,4);//subchunk1 id(str="fmt ")
    wav_file.read(buff,4);//subchunk1(fmt) size(int=16)
    wav_file.read(buff,2);//wav format=1(PCM)(int)

    wav_file.read(buff,2);//Channel Count(int=2)
    uint16_t channel_count = *((uint16_t *)buff);
    wav_file.read(buff,4);//Sample Rate(int=16K)
    uint16_t sample_rate = *((uint16_t *)buff);

    wav_file.read(buff,4);//Byte per sec(int, 64K=16*4)
    wav_file.read(buff,2);//Byte Per Block(int, 4(2*2))
    wav_file.read(buff,2);//Bit Per Sample(int, 16 bit)

    wav_file.read(buff,4);//subchunk2 id(str="data")
    wav_file.read(buff,4);//subchunk2 size(int=sample count)
    uint16_t data_size = *((uint16_t *)buff);
    //    qDebug() << "sample_rate:"  << sample_rate
    //             << "channel:" << channel_count
    //             << "chunk_size:" << data_size;
}

void BtEnn::readWav(BtCyclic *out)
{
    QVector<int16_t> data_buff;
    char buff[200];
    int i = 0;

    while( !wav_file.atEnd() )
    {
        i++;

        wav_file.read(buff, 2);
        data_buff.push_back(*((uint16_t *)buff));
        wav_file.read(buff, 2); // skip second channel
    }
    out->write(&data_buff);
}

void BtEnn::preProcess()
{
    for( int i=0 ; i<last_r.size() ; i++ )
    {
        last_r[i].start -= ENN_GAURD_TIME;
        last_r[i].end   += ENN_GAURD_TIME;

        if( last_r[i].start<0 )
        {
            last_r[i].start = 0;
        }
        double u_end = o_decoder->uframe/100.0;
        if( last_r[i].end>u_end )
        {
            last_r[i].end = u_end;
        }
    }
}

bool BtEnn::sanityCheck(QString filename)
{
    QFileInfo file_info(filename);
    QString fname = file_info.fileName();

    fname.remove(".wav");
    int dot_index = fname.indexOf(".");
    fname = fname.mid(0, dot_index);

    wav_w->readWordList();
    QString d_name = wav_w->wordToId(last_r); // d_name : detected name

    if( d_name==fname )
    {
        return true;
    }
    else
    {
        qDebug() << "d_name" << d_name << "fname" << fname;
        return false;
    }
}

void BtEnn::saveFeature(QString filename, BtCFB *cfb)
{
    QFileInfo file_info(filename);
    QString fname = file_info.fileName();

    fname.remove(".wav");

    for( int i=0 ; i<last_r.size() ; i++ )
    {
        QString path = enn_dir;
        path += last_r[i].word;
        bt_mkDir(path);
        path += "/" + cat_dir;
        path += "_" + fname;
        path += "_" + QString::number(i);

        int len = 100*(last_r[i].end - last_r[i].start);
        int start = 100*last_r[i].start + o_decoder->status.min_frame;

        //        qDebug() << "timing :" << len << start << 100*last_r[i].start;

        QVector<BtFrameBuf *> buffer;
        for( int j=0 ; j<len ; j++ )
        {
            BtFrameBuf *buf = cfb->get(start + j);
            buffer.push_back(buf);
        }
//        saveImage(path, buffer);
        writeSample(path, buffer);
        //        saveCSV(path, buffer);
    }
}

void BtEnn::saveImage(QString filename, QVector<BtFrameBuf *> data)
{
    int len = data.length();
    double sum = 0;
    QImage *img = new QImage(len, BT_ENN_SIZE, QImage::Format_RGB888);

    calcStat(data);
    for( int i=0 ; i<len ; i++ )
    {
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            double val = data[i]->enn[j];
            sum += val;
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

    QImage img_s = img->scaled(BT_ENN_SIZE, BT_ENN_SIZE);

    if( !img_s.save(filename + ".png", "PNG") )
    {
        qDebug() << "Error: saving image failed.";
    }
    //    qDebug() << "E: path: " << filename << max_delta[0] <<
    //    max_delta[1] << max_delta[2] << min_delta[0] << min_delta[1]
    //    << min_delta[2];
    //    qDebug() << "siza :" << len;
    QString cmd = "eog ";
    cmd += filename;
    cmd += ".png &";
    //    system(cmd.toStdString().c_str());
}

void BtEnn::writeSample(QString filename,
                        QVector<BtFrameBuf *> data)
{
    int len = data.length();
    QFile m_file(filename+".enn");
    if( !m_file.open(QFile::WriteOnly) )
    {
        qDebug() << "Could not open file for writing"
                 << filename;
        return;
    }

    QDataStream out(&m_file);
    out << BT_ENN_SIZE;
    out << BT_ENN_SIZE;
    double scale_val = (len-1.0)/(BT_ENN_SIZE-1.0);
    for( int f=0 ; f<BT_ENN_SIZE ; f++ ) // freq
    {
        for( int t=0 ; t<BT_ENN_SIZE ; t++ ) //time
        {
            double p = t*scale_val;
            int p_round = qRound(p);
            if( qAbs(p-p_round)<0.001 )
            {
                // if remove this line calc error would
                // cause segfault in p_left or p_right
                // in the marginal values
                p = p_round;
            }

            // calculate mean from left and right samples
            // in cepstrum because of scaling
            int p_right = qCeil(p);
            int p_left  = qFloor(p);
            double d = p-p_left;
            float val = d*data[p_right]->enn[f]
                    +(1-d)*data[p_left]->enn[f];
            double ds = enn_getDimScale(p, len);
//            val = 0.1*j- + i*4;

            out << val * ds;
        }
    }
    m_file.close();
}

void BtEnn::saveCSV(QString filename, QVector<BtFrameBuf *> data)
{
    QFile file;
    QTextStream out(&file);
    int len = data.length();

    file.setFileName(filename + ".csv");
    if( !file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        qDebug() << "Failed To Create" << filename;
        exit(1);
    }


    for( int i=0 ; i<len ; i++ )
    {
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            int val = data[i]->enn[j];
            val += offset_delta;
            out << QString::number(val);
            out << ",";
        }
        out << "\n";
    }

    file.close();
}

void BtEnn::saveWave(QString filename)
{
    double end = o_decoder->uframe/100.0;
    double rw_len = end - last_r[0].end;  // rewind length
    double word_len;
    double sil_len; // silence length

    QFileInfo file_info(filename);
    QString fname = file_info.fileName();
    fname.remove(".wav");

    rw_len *= BT_REC_RATE;
    cy_buf->rewind(rw_len);

    for( int i=0 ; i<last_r.size() ; i++ )
    {
        QString path = enn_dir;
        path += last_r[i].word;
        bt_mkDir(path);
        path += "/" + cat_dir;
        path += "_" + fname;
        path += "_" + QString::number(i);
        path += ".wav";

        word_len = last_r[i].end - last_r[i].start;
        word_len *= BT_REC_RATE;

        wav_w->writeEnn(path, word_len);

        if( i<last_r.size()-1 )
        {
            sil_len  = last_r[i+1].end - last_r[i].end;
            sil_len *= BT_REC_RATE;
            cy_buf->rewind(-sil_len); // minus for forwarding :D
        }
    }
}

bool BtEnn::checkExist(QString path)
{
    QFileInfo file_info(path);
    QString fname = file_info.fileName();

    fname.remove(".wav");

    for( int i=0 ; i<exist_list.size() ; i++ )
    {
        if( exist_list[i].contains(fname) )
        {
            //            qDebug() << exist_list[i] << fname;
            return true;
        }
    }

    return false;
}

void BtEnn::calcStat(QVector<BtFrameBuf *> data)
{
    int len = data.length();
    double var = 0;
    int N = len * BT_ENN_SIZE;

    double sum = 0;

    for( int i=0 ; i<len ; i++ )
    {
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            double val = data[i]->enn[j];
            sum += val;
            var += qPow(val, 2);
            //            qDebug() << "val" << val;

            if( max_delta[0]<data[i]->enn[j] )
            {
                max_delta[0] = data[i]->enn[j];
            }
            if( min_delta[0]>data[i]->enn[j] )
            {
                min_delta[0] = data[i]->enn[j];
            }
        }
    }
    double mean = sum/N;
    var = qSqrt(var/N - qPow(mean, 2));
    offset_delta = mean;
    scale_delta = var;
    //    qDebug() << "min_delta" << min_delta[0] << max_delta[0] << var << mean;
    //    exit(0);
}

void BtEnn::fillExist()
{
    QDir enn(enn_dir);
    QStringList words = enn.entryList(QDir::Dirs);

    int len = words.length();
    for( int i=0 ; i<len ; i++ )
    {
        QString path = enn_dir + words[i];
        exist_list += QDir(path).entryList(QDir::Files);
    }
}

void BtEnn::moveToShit(QString path)
{
    QString old_path = path;
    QFileInfo info(old_path);
    QFile file(old_path);

    QString new_path = shit_dir;
    new_path += info.fileName();
    file.copy(new_path);
//    file.remove();
}
