#include "bt_test.h"

using namespace kaldi;
using namespace fst;

BtTest::BtTest(QString filename, QObject *parent): QObject(parent)
{
    wav_file = NULL;
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);

    QDir p_dir(filename);
    QStringList fmt;
    fmt.append("*.wav");
    file_list = p_dir.entryList(fmt, QDir::Files);

    for( int i = 0 ; i<file_list.size() ; i++ )
    {
        file_list[i] = filename + file_list[i];
    }

    init();
}

BtTest::~BtTest()
{
    delete o2_model;
    delete o_decoder;
}

void BtTest::init()
{
    std::string model_filename = BT_OAMDL_PATH;
    o2_model = new KdModel(model_filename);
    o_decoder = new KdOnlineLDecoder(*(o2_model->t_model));

    startDecode();
}

void BtTest::startDecode()
{
    float acoustic_scale = 0.05;
    int chunk_size = 16000; // 1000ms

    KdDecodable decodable(NULL, o2_model,
                             acoustic_scale);

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;
    QVector<BtWord> result;

//    qDebug() << file_list.size();
    for( int i = 0 ; i<10 ; i++ )
    {
        openWave(file_list[i]);
        int read_size = chunk_size;
        while( read_size==chunk_size )
        {
            read_size = readWav(chunk_size, cy_buf);
            decodable.features->AcceptWaveform(cy_buf);
            o_decoder->Decode();
            result = o_decoder->getResult(&out_fst);
        }
        if( result.size() )
        {
                bt_writeBarResult(result);
            }
            qDebug() << buf;
        }
        o_decoder->wav_id++;
        o_decoder->status.min_frame = o_decoder->frame_num;
        o_decoder->status.max_frame = 0;
        o_decoder->ResetDecoder(); // this reset uframe
        o_decoder->status.state = KD_STATE_NORMAL;
    }
    exit(0);

}

void BtTest::openWave(QString filename)
{
    wav_file = new QFile(filename);
    if( !wav_file->open(QIODevice::ReadWrite) )
    {
        qDebug() << "Failed To Open" << filename;
        exit(1);
    }
    qDebug() << QFileInfo(filename).fileName();

    char buff[200];

    wav_file->read(4); // ="RIFF" is the father of wav
    wav_file->read(buff,4);//chunk size(int)
    wav_file->read(buff,4);//format="WAVE"
    wav_file->read(buff,4);//subchunk1 id(str="fmt ")
    wav_file->read(buff,4);//subchunk1(fmt) size(int=16)
    wav_file->read(buff,2);//wav format=1(PCM)(int)

    wav_file->read(buff,2);//Channel Count(int=2)
    uint16_t channel_count = *((uint16_t *)buff);
    wav_file->read(buff,4);//Sample Rate(int=16K)
    uint32_t sample_rate = *((uint32_t *)buff);

    wav_file->read(buff,4);//Byte per sec(int, 64K=16*4)
    wav_file->read(buff,2);//Byte Per Block(int, 4(2*2))
    wav_file->read(buff,2);//Bit Per Sample(int, 16 bit)

    wav_file->read(buff,4);//subchunk2 id(str="data")
    wav_file->read(buff,4);//subchunk2 size(int=sample count)
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

    while( !wav_file->atEnd() )
    {
        i++;
        if( i>count )
        {
            break;
        }

        wav_file->read(buff, 2);
        data_buff.push_back(*((uint16_t *)buff));
        wav_file->read(buff, 2); // skip second channel
    }
    out->write(&data_buff);
    return i-1;
}
