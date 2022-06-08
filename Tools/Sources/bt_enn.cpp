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

    init();
    for( int i=0 ; i<BT_DELTA_ORDER+1 ; i++)
    {
        max_delta[i] = 0;
        min_delta[i] = 10000;
    }
}

BtEnn::~BtEnn()
{
    delete t_model;
    delete oa_model;
    delete o_decoder;
    delete cy_buf;
}

void BtEnn::init()
{
    std::string model_filename = BT_OAMDL_PATH;

    oa_model = new KdAModel;
    t_model = new kaldi::TransitionModel;

    bool binary;
    kaldi::Input ki(model_filename, &binary);
    t_model->Read(ki.Stream(), binary);
    oa_model->Read(ki.Stream(), binary);

    o_decoder = new KdOnlineLDecoder(t_model);

    startDecode();
}

void BtEnn::startDecode()
{
    float acoustic_scale = 0.05;
    int chunk_size = 16000; // 1000ms

    KdDecodable decodable(cy_buf, oa_model,
                          t_model, acoustic_scale);

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;
    QVector<BtWord> result;

//    qDebug() << file_list.size();
    for( int i = 0 ; i<4 ; i++ )
    {
        openWave(file_list[i]);
        int read_size = chunk_size;
        while( read_size==chunk_size )
        {
            read_size = readWav(chunk_size, cy_buf);
            decodable.features->ComputeFeatures();
            o_decoder->Decode();
            result = o_decoder->getResult(&out_fst);
            if( result.size()>=last_r.size() )
            {
                last_r = result;
            }
        }
        saveFeature(file_list[i], decodable.features->o_features);
        o_decoder->wav_id++;
        o_decoder->status.min_frame = o_decoder->frame_num;
        o_decoder->status.max_frame = 0;
        o_decoder->ResetDecoder(); // this reset uframe
        o_decoder->status.state = KD_STATE_NORMAL;
        last_r.clear();
    }
    exit(0);

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
    qDebug() << ">>>>" << QFileInfo(filename).fileName();

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

int BtEnn::readWav(int count, BtCyclic *out)
{
    QVector<int16_t> data_buff;
    char buff[200];
    int i = 0;

    while( !wav_file.atEnd() )
    {
        i++;
        if( i>count )
        {
            break;
        }

        wav_file.read(buff, 2);
        data_buff.push_back(*((uint16_t *)buff));
        wav_file.read(buff, 2); // skip second channel
    }
    out->write(&data_buff);
    return i-1;
}

void BtEnn::saveFeature(QString filename, BtCFB *cfb)
{
    QFileInfo file_info(filename);
    QString fname = file_info.fileName();

    fname.remove(".wav");

    for( int i=0 ; i<last_r.size() ; i++ )
    {
        QString path = KAL_AU_DIR"enn/online/";
        path += fname;
        path += "_" + last_r[i].word;
        path += "_" + QString::number(i);

        int len = 100*(last_r[i].end - last_r[i].start);
        int start = 100*last_r[i].start + o_decoder->status.min_frame;

        qDebug() << "timing :" << len << start << 100*last_r[i].start;

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
    QImage *img = new QImage(len, BT_DELTA_SIZE, QImage::Format_Grayscale8);

    for( int i=0 ; i<len ; i++ )
    {
        for( int j=0 ; j<BT_DELTA_SIZE ; j++ )
        {
            int val = data[i]->delta[j] + offset_delta[j/BT_FEAT_SIZE];
            val *= scale_delta[j/BT_FEAT_SIZE];
            img->setPixel(i, j, qRgb(val,val,val));
            if( max_delta[j/BT_FEAT_SIZE]<data[i]->delta[j] )
            {
                max_delta[j/BT_FEAT_SIZE] = data[i]->delta[j];
            }
            if( min_delta[j/BT_FEAT_SIZE]>data[i]->delta[j] )
            {
                min_delta[j/BT_FEAT_SIZE] = data[i]->delta[j];
            }
        }
    }

    QImage img_sag = img->scaled( 500, 390);

    if( !img_sag.save(filename + ".png", "PNG") )
    {
        qDebug() << "Error: saving image failed.";
    }
    qDebug() << "E: path: " << filename << max_delta[0] << max_delta[1] << max_delta[2] << min_delta[0] << min_delta[1] << min_delta[2];
    qDebug() << "siza :" << len;
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
        for( int j=0 ; j<BT_DELTA_SIZE ; j++ )
        {
            int val = data[i]->delta[j] + 270.0;
            val *= 3;
            out << QString::number(val);
            out << ",";
        }
        out << "\n";
    }

    file.close();
}
