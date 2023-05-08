#include "ab_wav_reader.h"

AbWavReader::AbWavReader()
{
    ab_checkAuDir("unverified");
    ab_checkAuDir("shit");
    ab_checkAuDir("train");

    wav_file = new QFile;
}

AbWavReader::~AbWavReader()
{
    ;
}

double AbWavReader::getPower(QString filename)
{
    double power = 0;
    wav_file->setFileName(filename);

    if( !wav_file->open(QIODevice::ReadOnly) )
    {
        qDebug() << "Failed To open" << filename;
        exit(1);
    }

    power = calcPower();

    wav_file->close();
    return power;
}

double AbWavReader::calcPower()
{
    readHeader();

    double power_dB = 0;
    int16_t dummy = 0;
    int16_t  value;
    double sum_sq = 0;
    int len = 0;

    while( !wav_file->atEnd() )
    {
        len++;
        // kaldi should be 2 channel
        wav_file->read((char *)&value, 2);
        wav_file->read((char *)&dummy, 2);
        sum_sq += pow(value,2);
    }

    if( len>0 )
    {
        double wav_power = sqrt(sum_sq)/len;
        power_dB = 20*log10(wav_power);
        power_dB += 50; // calibration
    }
    return power_dB;
}

void AbWavReader::readHeader()
{
    QByteArray read_data = wav_file->read(4); // "RIFF" is the father of wav
    if( read_data!="RIFF" )
    {
        qDebug() << "Error: wav file not started with RIFF";
    }

    int len;
    wav_file->read((char*)&len, 4);
    len = len - 44 + 8; // 44=Header Size - chunk size(int=filesize-8)
    wave_time = (double)len / BT_REC_RATE / 4; // 2 channel * 2 byte per sample
    read_data = wav_file->read(4); //format="WAVE"

    if( read_data!="WAVE" )
    {
        qDebug() << "Error: wav format is not WAVE";
    }

    read_data = wav_file->read(4); //subchunk1 id(str="fmt ")

    if( read_data!="fmt " )
    {
        qDebug() << "Error: subchunk1 problem, not started with fmt " << read_data;
    }

    uint32_t buf_i;
    uint16_t buf_s; //short
    wav_file->read((char*)&buf_i, 4); //subchunk1(fmt) size(int=16)
    wav_file->read((char*)&buf_s, 2); //wav format(int) 1=PCM

    if( buf_i!=16 || buf_s!=1 )
    {
        qDebug() << "Error: subchunk size problem:"
                 << buf_i << "and format is not PCM:"
                 << buf_s;
    }

    //channel must be stereo for kaldi
    wav_file->read((char*)&buf_s, 2); //Channel Count(int=2)
    wav_file->read((char*)&buf_i, 4); //Sample Rate(int=16K)

    if( buf_i!=16000 || buf_s!=2 )
    {
        qDebug() << "Error: sample rate problem:"
                 << buf_i << "and channel count problem:"
                 << buf_s;
    }

    wav_file->read((char*)&buf_i, 4); //Byte per sec(int, 64K=16*4)
    wav_file->read((char*)&buf_s, 2); //Byte Per Block(int, 4=2ch*2)

    if( buf_i!=64000 || buf_s!=4 )
    {
        qDebug() << "Error: Byte per sec problem:"
                 << buf_i << "and Byte Per Block problem:"
                 << buf_s;
    }

    wav_file->read((char*)&buf_s, 2); //Bit Per Sample(int, 16 bit)

    if( buf_s!=16 )
    {
        qDebug() << "Error: Bit Per Sample problem:"
                 << buf_s;
    }

    read_data = wav_file->read(4);//subchunk2 id(str="data")

    if( read_data!="data" )
    {
        qDebug() << "Error: subchunk2 is not started with data.";
    }

    wav_file->read((char*)&buf_i, 4);//subchunk2 size(int=sample count)
}
