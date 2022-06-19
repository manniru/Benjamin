#include "bt_test.h"
#include <QDir>

BtTest::BtTest(QString dir_name, QObject *parent): QObject(parent)
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

    net = new BtNetwork;
    init();
}

BtTest::~BtTest()
{
    delete t_model;
    delete oa_model;
    delete o_decoder;
    delete cy_buf;
}

void BtTest::init()
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

void BtTest::startDecode()
{
    float acoustic_scale = 0.05;
    int chunk_size = 16000; // 1000ms

    KdDecodable decodable(cy_buf, oa_model,
                          t_model, acoustic_scale);
    net->cfb   = decodable.features->o_features;
    net->wav_w = new BtWavWriter(cy_buf);
    decodable.features->enableENN();

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;
    QVector<BtWord> result;

//    qDebug() << file_list.size();
    for( int i = 0 ; i<2 ; i++ )
    {
        openWave(file_list[i]);
        int read_size = chunk_size;
        while( read_size==chunk_size )
        {
            read_size = readWav(chunk_size, cy_buf);
            decodable.features->ComputeFeatures();
            o_decoder->Decode();
            result = o_decoder->getResult(&out_fst);
        }
        if( result.size() )
        {
            QString buf;
            for( int i=0 ; i<result.size() ; i++ )
            {
                buf += result[i].word;
                buf += " ";
                bt_writeBarResult(result);
            }
            qDebug() << buf;
        }
        o_decoder->wav_id++;
        o_decoder->resetODecoder();
    }
    exit(0);

}

void BtTest::openWave(QString filename)
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
    qDebug() << QFileInfo(filename).fileName();

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

int BtTest::readWav(int count, BtCyclic *out)
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

float BtTest::getConf(BtWord word)
{
    int len = 100*(word.end - word.start);
    float conf = net->getConf(word.stf, len, word.word_id);
    return conf;
}

void BtTest::saveWave(int start, int len, QString word)
{
    double end = o_decoder->uframe/100.0;
    double rw_len = end - (start + len); // rewind length
    double word_len;

    QString fname = KAL_AU_DIR"tt/";
    bt_mkDir(fname);
    fname = word + ".wav";

    rw_len *= BT_REC_RATE/1000.0;
    cy_buf->rewind(rw_len);
    word_len = len * BT_REC_RATE/1000.0;

    net->wav_w->writeEnn(fname, word_len);
}
