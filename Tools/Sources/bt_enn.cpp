#include "bt_enn.h"

BtEnn::BtEnn(QString dir_name, QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);

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

    mkDir(KAL_AU_DIR"enn/");

    QString cmd = "find " KAL_AU_DIR "enn/ -type f";
    exist_list = getStrCommand(cmd).split("\n");
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
    std::string model_filename = BT_OAMDL_PATH;

    oa_model = new KdAModel;
    t_model = new kaldi::TransitionModel;

    bool binary;
    kaldi::Input ki(model_filename, &binary);
    t_model->Read(ki.Stream(), binary);
    oa_model->Read(ki.Stream(), binary);

    o_decoder = new KdOnlineLDecoder(t_model);
    o_decoder->status.min_sil = 150;

    startDecode();
}

void BtEnn::startDecode()
{
    float acoustic_scale = 0.05;

    KdDecodable decodable(cy_buf, oa_model,
                          t_model, acoustic_scale);
    decodable.features->enableENN();

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;

//    int len = 100;
    int len = file_list.size();
//    qDebug() << file_list.size() << cat_dir;
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
        saveFeature(file_list[i], decodable.features->o_features);
        o_decoder->wav_id++;
        o_decoder->resetODecoder();
        last_r.clear();
    }
}

void BtEnn::openWave(QString filename)
{
    if( wav_file.isOpen() )
    {
        wav_file.close();
    }

    wav_file.setFileName(filename);
    if( !wav_file.open(QIODevice::ReadWrite) )
    {
        qDebug() << "Failed To Open" << filename;
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
    uint32_t sample_rate = *((uint32_t *)buff);

    wav_file.read(buff,4);//Byte per sec(int, 64K=16*4)
    wav_file.read(buff,2);//Byte Per Block(int, 4(2*2))
    wav_file.read(buff,2);//Bit Per Sample(int, 16 bit)

    wav_file.read(buff,4);//subchunk2 id(str="data")
    wav_file.read(buff,4);//subchunk2 size(int=sample count)
    uint16_t data_size = *((uint32_t *)buff);
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

void BtEnn::saveFeature(QString filename, BtCFB *cfb)
{
    QFileInfo file_info(filename);
    QString fname = file_info.fileName();

    fname.remove(".wav");

    for( int i=0 ; i<last_r.size() ; i++ )
    {
        QString path = KAL_AU_DIR"enn/";
        path += last_r[i].word;
        mkDir(path);
        path += "/" + cat_dir;
        path += "_" + fname;
        path += "_" + QString::number(i);

        int len = 100*(last_r[i].end - last_r[i].start);
        int start = 100*last_r[i].start + o_decoder->status.min_frame;

//        qDebug() << "timing :" << len << start << 100*last_r[i].start;

        QVector <BtFrameBuf *> buffer;
        for( int j=0 ; j<len ; j++ )
        {
            BtFrameBuf *buf = cfb->get(start + j);
            buffer.push_back(buf);
        }
        saveImage(path, buffer);
//        saveCSV(path, buffer);
    }
}

void BtEnn::saveImage(QString filename, QVector<BtFrameBuf *> data)
{
    int len = data.length();
    double sum = 0;
    QImage *img = new QImage(len, BT_DELTA_SIZE, QImage::Format_RGB888);

    for( int i=0 ; i<len ; i++ )
    {
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            double val = data[i]->enn[j];
            sum += val;
            val += offset_delta;
            val /= scale_delta;
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
    calcStat(data, sum);

    QImage img_sag = img->scaled(500, 390);

    if( !img_sag.save(filename + ".png", "PNG") )
    {
        qDebug() << "Error: saving image failed.";
    }
//    qDebug() << "E: path: " << filename << max_delta[0] << max_delta[1] << max_delta[2] << min_delta[0] << min_delta[1] << min_delta[2];
//    qDebug() << "siza :" << len;
    QString cmd = "eog ";
    cmd += filename;
    cmd += ".png &";
//    system(cmd.toStdString().c_str());
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

void BtEnn::mkDir(QString path)
{
    QDir au_EnnDir(path);

    if( !au_EnnDir.exists() )
    {
        qDebug() << "Creating" << path
                 << " Directory";
        QString command = "mkdir -p ";
        command += path;
        system( command.toStdString().c_str() );
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

void BtEnn::calcStat(QVector<BtFrameBuf *> data, double sum)
{
    int len = data.length();
    double mean = sum/len/BT_ENN_SIZE;
    double var = 0;

    for( int i=0 ; i<len ; i++ )
    {
        for( int j=0 ; j<BT_ENN_SIZE ; j++ )
        {
            double val = data[i]->enn[j];
            var += qPow(val-mean, 2);
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
    var = qSqrt(var/len/BT_ENN_SIZE);
//    qDebug() << "min_delta" << min_delta[0] << max_delta[0] << var << mean;
//    exit(0);
}
