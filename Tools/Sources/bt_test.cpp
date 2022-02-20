#include "bt_test.h"

using namespace kaldi;
using namespace fst;


BtTest::BtTest(QString filename, QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);
    readWav(filename, cy_buf);
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
    QVector<int> silence_phones = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    TransitionModel *trans_model = new TransitionModel;
    AmDiagGmm       *am_gmm = new AmDiagGmm;

    bool rx_binary;
    Input ki(model_rxfilename, &rx_binary);
    trans_model->Read(ki.Stream(), rx_binary);
    am_gmm->Read(ki.Stream(), rx_binary);

    std::string online_alimdl = KAL_NATO_DIR"exp/tri1_online/final.oalimdl";
    o2_model = new KdOnline2Model(trans_model, am_gmm, online_alimdl);

    o_decoder = new KdOnlineLDecoder(silence_phones, *trans_model);
    startDecode();
}

void BtTest::startDecode()
{
    float acoustic_scale = 0.05;

    KdOnline2Decodable decodable(NULL, o2_model,
                             acoustic_scale);

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;
    QVector<BtWord> result;

    decodable.features->AcceptWaveform(cy_buf);
    o_decoder->Decode();
    clock_t start_t = clock();
    result = o_decoder->getResult(&out_fst);
    bt_writeBarResult(result);

    if( result.size() )
    {
        for( int i=0 ; i<result.size() ; i++ )
        {
            qDebug() << result[i].word
                     << result[i].start << result[i].end;
        }
        qDebug() << getDiffTime(start_t);
    }

    qDebug() << "Sorting Started" << decodable.NumFramesReady();
    for( int i=0 ; i<500 ; i++ )
    {
//        qDebug() << i << decodable.p_vec[i].phone_id;
    }
//    qDebug() << decodable.p_vec.size();

    if( o_decoder->status.state==KD_STATE_SILENCE )
    {
        status.last_word = "";
        status.word_count = 0;
    }
}

void BtTest::readWav(QString filename, BtCyclic *out)
{
    QFile m_WAVFile;
    m_WAVFile.setFileName(filename);
    m_WAVFile.open(QIODevice::ReadWrite);

    char buff[200];
    QVector<int16_t> data_buff;

    m_WAVFile.read(4); // ="RIFF" is the father of wav
    m_WAVFile.read(buff,4);//chunk size(int)
    m_WAVFile.read(buff,4);//format=WAVE(str)
    m_WAVFile.read(buff,4);//subchunk1 id(str)
    m_WAVFile.read(buff,4);//subchunk1 size(int)
    m_WAVFile.read(buff,2);//wav format=1(PCM)(int)

    m_WAVFile.read(buff,2);
    uint16_t channel_count = *((uint16_t *)buff);
    m_WAVFile.read(buff,4);
    uint32_t sample_rate = *((uint32_t *)buff);

    m_WAVFile.read(buff,4);//Byte rate(int, 64K=16*4)

    m_WAVFile.read(buff,2);//Block Allign(int, 4(2*2))
    m_WAVFile.read(buff,2);//BitPerSample(int, 16 bit)
    qDebug() << "sample_rate:"  << sample_rate
             << "channel:" << channel_count;

    m_WAVFile.read(buff,4);//subchunk2 id(str=data)
    m_WAVFile.read(buff,4);//subchunk2 size(int)
    uint16_t chunk_size = *((uint32_t *)buff);
    int i = 0;

    while( !m_WAVFile.atEnd() )
    {
        i++;
        m_WAVFile.read(buff, 2);
        data_buff.push_back(*((uint16_t *)buff));
        m_WAVFile.read(buff, 2); // skip second channel
    }

    qDebug() << "i:" << i << chunk_size;
    out->write(&data_buff);
}
