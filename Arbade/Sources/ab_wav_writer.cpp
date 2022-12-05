#include "ab_wav_writer.h"
#include <QDir>

AbWavWriter::AbWavWriter(int16_t *buffer, int sample_count)
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

AbWavWriter::~AbWavWriter()
{
    ;
}

void AbWavWriter::write(QString filename)
{
    file->setFileName(filename);

    if( !file->open(QIODevice::WriteOnly) )
    {
        qDebug() << "Failed To Create" << filename;
        exit(1);
    }

    writeWav();
    file->close();
}

void AbWavWriter::writeWav()
{
    writeWavHeader(buf_size*4); // 2 channel * 2 byte per sample
    uint16_t zero = 0;
    for( int i=0 ; i<buf_size ; i++ )
    {
        int16_t *pt = &cy_buf[i];
        // kaldi should be 2 channel
        file->write((char *)pt, 2);
        file->write((char *)&zero, 2);
    }
}

bool AbWavWriter::isSleep()
{
    QString status_path = getenv("HOME");
    status_path += "/.config/polybar/awesomewm/ben_status";
    if( QFile::exists(status_path) )
    {
        return true;
    }
    return false;
}

void AbWavWriter::writeWavHeader(int len)
{
    uint32_t buf_i;
    uint16_t buf_s; //short

    file->write("RIFF", 4); // "RIFF" is the father of wav
    buf_i = len + 44 - 8; // 44=Header Size
    file->write((char*)&buf_i,4);//chunk size(int=filesize-8)
    file->write("WAVE",4);//format="WAVE"
    file->write("fmt ",4);//subchunk1 id(str="fmt ")
    buf_i = 16; file->write((char*)&buf_i,4);//subchunk1(fmt) size(int=16)
    buf_s = 1;  file->write((char*)&buf_s,2);//wav format(int) 1=PCM

    //channel must be stereo for kaldi
    buf_s = 2;     file->write((char*)&buf_s,2);//Channel Count(int=2)
    buf_i = 16000; file->write((char*)&buf_i,4);//Sample Rate(int=16K)

    buf_i = 64000; file->write((char*)&buf_i,4);//Byte per sec(int, 64K=16*4)
    buf_s = 4;     file->write((char*)&buf_s,2);//Byte Per Block(int, 4=2ch*2)
    buf_s = 16;    file->write((char*)&buf_s,2);//Bit Per Sample(int, 16 bit)

    file->write("data",4);//subchunk2 id(str="data")
    buf_i = len;
    file->write((char*)&buf_i,4);//subchunk2 size(int=sample count)
//    qDebug() << "sample_rate:"  << buf_i;
}

void AbWavWriter::setCategory(QString cat)
{
    category = cat;

    QString base_name = KAL_AU_DIR_WIN"train\\";
    base_name += category + "\\";
    QDir au_OnlineDir(base_name);

    if( !au_OnlineDir.exists() )
    {
        qDebug() << "Creating" << base_name
                 << " Directory";
        QString cmd;
#ifdef WIN32
        cmd = "mkdir " + base_name;
        system(cmd.toStdString().c_str());
#else //OR __linux
        cmd = "mkdir -p " KAL_AU_DIR "online";
        system(cmd.toStdString().c_str());
#endif
    }
}
