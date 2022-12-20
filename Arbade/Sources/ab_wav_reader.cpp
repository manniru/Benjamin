#include "ab_wav_reader.h"

AbWavReader::AbWavReader(int16_t *buffer, int sample_count)
{
    cy_buf = buffer;
    buf_size = sample_count;

    QDir au_UnverifiedDir(KAL_AU_DIR"unverified");

    if( !au_UnverifiedDir.exists() )
    {
        qDebug() << "Creating" << KAL_AU_DIR"unverified"
                 << " Directory";
#ifdef WIN32
        system("mkdir " KAL_AU_DIR_WIN "unverified");
#else //OR __linux
        system("mkdir -p " KAL_AU_DIR "unverified");
#endif
    }

    QDir au_TrainDir(KAL_AU_DIR"train/online");

    if( !au_TrainDir.exists() )
    {
        qDebug() << "Creating" << KAL_AU_DIR"train/online"
                 << " Directory";
#ifdef WIN32
        system("mkdir " KAL_AU_DIR_WIN "train\\online");
#else //OR __linux
        system("mkdir -p " KAL_AU_DIR "train/online");
#endif
    }

    file = new QFile;

    exemption_list << "kick";
    exemption_list << "side";
    exemption_list << "copy";
    exemption_list << "paste";
}

AbWavReader::~AbWavReader()
{
    ;
}

void AbWavReader::read(QString filename)
{
    file->setFileName(filename);

    if( !file->open(QIODevice::ReadOnly) )
    {
        qDebug() << "Failed To open" << filename;
        exit(1);
    }

    readWav();
    file->close();
}

void AbWavReader::readWav()
{
    readWavHeader();
    uint16_t zero = 0;
    double sum_sq = 0;
    for( int i=0 ; i<buf_size ; i++ )
    {
        int16_t pt;
        // kaldi should be 2 channel
        file->read((char *)&pt, 2);
        file->read((char *)&zero, 2);
        sum_sq += pow(pt,2);
    }
    double wav_power = sqrt(sum_sq)/buf_size;
    power_dB = 20*log10(wav_power);
    power_dB += 50; // calibration
}

void AbWavReader::readWavHeader()
{
    QByteArray read_data = file->read(4); // "RIFF" is the father of wav
    if( read_data!="RIFF" )
    {
        qDebug() << "Error: wav file not started with RIFF";
    }
    int len;
    file->read((char*)&len, 4);
    len = len - 44 + 8; // 44=Header Size - chunk size(int=filesize-8)
    wave_time = (double)len/BT_REC_RATE/4; // 2 channel * 2 byte per sample
    read_data = file->read(4); //format="WAVE"
    if( read_data!="WAVE" )
    {
        qDebug() << "Error: wav format is not WAVE";
    }
    read_data = file->read(4); //subchunk1 id(str="fmt ")
    if( read_data!="fmt " )
    {
        qDebug() << "Error: subchunk1 problem, not started with fmt " << read_data;
    }
    uint32_t buf_i;
    uint16_t buf_s; //short
    file->read((char*)&buf_i,4);//subchunk1(fmt) size(int=16)
    file->read((char*)&buf_s,2);//wav format(int) 1=PCM
    if( buf_i!=16 || buf_s!=1 )
    {
        qDebug() << "Error: subchunk size problem:"
                 << buf_i << "and format is not PCM:"
                 << buf_s;
    }

    //channel must be stereo for kaldi
    file->read((char*)&buf_s,2);//Channel Count(int=2)
    file->read((char*)&buf_i,4);//Sample Rate(int=16K)
    if( buf_i!=16000 || buf_s!=2 )
    {
        qDebug() << "Error: sample rate problem:"
                 << buf_i << "and channel count problem:"
                 << buf_s;
    }

    file->read((char*)&buf_i,4);//Byte per sec(int, 64K=16*4)
    file->read((char*)&buf_s,2);//Byte Per Block(int, 4=2ch*2)
    if( buf_i!=64000 || buf_s!=4 )
    {
        qDebug() << "Error: Byte per sec problem:"
                 << buf_i << "and Byte Per Block problem:"
                 << buf_s;
    }
    file->read((char*)&buf_s,2);//Bit Per Sample(int, 16 bit)
    if( buf_s!=16 )
    {
        qDebug() << "Error: Bit Per Sample problem:"
                 << buf_s;
    }

    read_data = file->read(4);//subchunk2 id(str="data")
    if( read_data!="data" )
    {
        qDebug() << "Error: subchunk2 is not started with data.";
    }
    file->read((char*)&buf_i, 4);//subchunk2 size(int=sample count)
}

void AbWavReader::setCategory(QString cat)
{
    category = cat;

    QString base_name = KAL_AU_DIR_WIN"unverified\\";
    QDir au_verDir(base_name);

    if( !au_verDir.exists() )
    {
        qDebug() << "Creating" << base_name
                 << " Directory";
        QString cmd;
#ifdef WIN32
        cmd = "mkdir " + base_name;
        system(cmd.toStdString().c_str());
#else //OR __linux
        cmd = "mkdir -p " KAL_AU_DIR "unverified";
        system(cmd.toStdString().c_str());
#endif
    }
}
