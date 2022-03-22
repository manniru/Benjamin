#include "bt_test.h"

using namespace kaldi;
using namespace fst;

BtTest::BtTest(QString filename, QObject *parent): QObject(parent)
{
    wav_file = NULL;
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);
    openWave(filename);
    init();
}

BtTest::~BtTest()
{
    delete o2_model;
    delete o_decoder;
}

void BtTest::init()
{
    status.word_count = 0;

    std::string model_rxfilename = BT_OAMDL_PATH;

    TransitionModel *trans_model = new TransitionModel;
    AmDiagGmm       *am_gmm = new AmDiagGmm;

    bool rx_binary;
    Input ki(model_rxfilename, &rx_binary);
    trans_model->Read(ki.Stream(), rx_binary);
    am_gmm->Read(ki.Stream(), rx_binary);

    std::string online_alimdl = KAL_NATO_DIR"exp/tri1_online/final.oalimdl";
    o2_model = new KdOnline2Model(trans_model, am_gmm, online_alimdl);

    o_decoder = new KdOnlineLDecoder(*trans_model);
    startDecode();
}

void BtTest::startDecode()
{
    float acoustic_scale = 0.05;
    int chunk_size = 1600; // 1000ms

    KdDecodable decodable(NULL, o2_model,
                             acoustic_scale);

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;
    QVector<BtWord> result;

    int read_size = chunk_size;
    while( read_size==chunk_size )
    {
        read_size = readWav(chunk_size, cy_buf);
        decodable.features->AcceptWaveform(cy_buf);
        o_decoder->Decode();
        result = o_decoder->getResult(&out_fst);
        if( result.size() )
        {
            bt_writeBarResult(result);
        }

        if( o_decoder->status.state!=KD_STATE_NORMAL )
        {
            status.word_count = 0;
        }
//        exit(0);
    }
}

void BtTest::openWave(QString filename)
{
    wav_file = new QFile;
    wav_file->setFileName(filename);
    if( !wav_file->open(QIODevice::ReadWrite) )
    {
        qDebug() << "Failed To Open" << filename;
        exit(1);
    }

    char buff[200];

    wav_file->read(4); // ="RIFF" is the father of wav
    wav_file->read(buff,4);//chunk size(int)
    wav_file->read(buff,4);//format=WAVE(str)
    wav_file->read(buff,4);//subchunk1 id(str)
    wav_file->read(buff,4);//subchunk1 size(int)
    wav_file->read(buff,2);//wav format=1(PCM)(int)

    wav_file->read(buff,2);
    uint16_t channel_count = *((uint16_t *)buff);
    wav_file->read(buff,4);
    uint32_t sample_rate = *((uint32_t *)buff);

    wav_file->read(buff,4);//Byte rate(int, 64K=16*4)

    wav_file->read(buff,2);//Block Allign(int, 4(2*2))
    wav_file->read(buff,2);//BitPerSample(int, 16 bit)

    wav_file->read(buff,4);//subchunk2 id(str=data)
    wav_file->read(buff,4);//subchunk2 size(int)
    uint16_t chunk_size = *((uint32_t *)buff);
    qDebug() << "sample_rate:"  << sample_rate
             << "channel:" << channel_count
             << "chunk_size:" << chunk_size;
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
