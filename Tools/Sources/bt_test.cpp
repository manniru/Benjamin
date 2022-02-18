#include "bt_test.h"

BtTest::BtTest(QString filename, QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);
    readWav(filename, cy_buf);
}

BtTest::~BtTest()
{

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
