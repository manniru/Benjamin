#include "bt_wav_writer.h"

BtWavWriter::BtWavWriter(BtCyclic *buffer, QObject *parent): QObject(parent)
{
    cy_buf = buffer;
    QDir au_OnlineDir(KAL_AU_DIR"/online");

    if( !au_OnlineDir.exists() )
    {
        qDebug() << "Creating" << KAL_AU_DIR"/online"
                 << " Directory";
        system("mkdir -p " KAL_AU_DIR "/online");
    }

    QDir au_TrainDir(KAL_AU_DIR"/train/online");

    if( !au_TrainDir.exists() )
    {
        qDebug() << "Creating" << KAL_AU_DIR"/train/online"
                 << " Directory";
        system("mkdir -p " KAL_AU_DIR "/train/online");
    }
    file = new QFile;
}

BtWavWriter::~BtWavWriter()
{
    ;
}

void BtWavWriter::write(QVector<BtWord> result, int len, int dbg_id)
{
    QString filename = KAL_AU_DIR"/online/";
    filename += QString::number(dbg_id);
    filename += ".wav";
    file->setFileName(filename);

    if( !file->open(QIODevice::WriteOnly) )
    {
        qDebug() << "Failed To Create" << filename;
        exit(1);
    }

    writeWav(len);
    file->close();

    copyToTrain(result, filename);
}

void BtWavWriter::writeWav(int len)
{
    int16_t *data = (int16_t *)malloc(len*sizeof(int16_t));

    int rew = cy_buf->rewind(len);
    if( rew==0 )
    {
        qDebug() << "Error 137: Failed to write wav, long len"
                 << len;
        free(data);
        return;
    }
    cy_buf->read(data, len);

    writeWavHeader(len*4); // 2 channel * 2 byte per sample
    uint16_t zero = 0;
    for( int i=0 ; i<len ; i++ )
    {
        int16_t *pt = &data[i];
        // kaldi should be 2 channel
        file->write((char *)pt, 2);
        file->write((char *)&zero, 2);
    }
    free(data);
}

bool BtWavWriter::isSleep()
{
    QString status_path = getenv("HOME");
    status_path += "/.config/polybar/awesomewm/ben_status";
    if( QFile::exists(status_path) )
    {
        return true;
    }
    return false;
}

void BtWavWriter::writeWavHeader(int len)
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

void BtWavWriter::copyToTrain(QVector<BtWord> result, QString filename)
{
    QString base_name = KAL_AU_DIR"/train/online/";
    for( int i=0 ; i<result.size()-1 ; i++ )
    {
        base_name += result[i].word;
        base_name += "_";
    }
    base_name += result.last().word;
    QString f_name = base_name + ".wav";

    int ps = 0;
    while( QFile::exists(f_name) )
    {
        ps++;
        f_name  = base_name + "_";
        f_name += QString::number(ps);
        f_name += ".wav";
    }

    qDebug() << "tr_filename" << f_name;

    QString cmd = "cp "+ filename;
    cmd += " " + f_name;
    system(cmd.toStdString().c_str());
}
