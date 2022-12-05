#include "bt_wav_writer.h"
#include <QDir>

BtWavWriter::BtWavWriter(int16_t *buffer, int sample_count)
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

    readWordList();
    exemption_list << "kick";
    exemption_list << "side";
    exemption_list << "copy";
    exemption_list << "paste";
}

BtWavWriter::~BtWavWriter()
{
    ;
}

void BtWavWriter::write(QVector<BtWord> result)
{
    QString filename = calcFileName(result);
    file->setFileName(filename);

    if( !file->open(QIODevice::WriteOnly) )
    {
        qDebug() << "Failed To Create" << filename;
        exit(1);
    }

    writeWav();
    file->close();
}

void BtWavWriter::writeWav()
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

void BtWavWriter::readWordList()
{
    if( word_list.size() )
    {
        return;
    }

    QFile wl_file(BT_WORDLIST_PATH);

    if( !wl_file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << BT_WORDLIST_PATH;
        return;
    }

    while ( !wl_file.atEnd() )
    {
        QString line = QString(wl_file.readLine());
        line.remove(QRegExp("\\n")); //remove new line
        if( line.length() )
        {
            word_list.push_back(line);
        }
    }

    wl_file.close();
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

void BtWavWriter::setCategory(QString cat)
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


QString BtWavWriter::calcFileName(QVector<BtWord> result)
{
    // verified base name
    QString base_name = KAL_AU_DIR"train/";
    base_name += category + "/";
    base_name += wordToId(result);
    QString name = base_name + ".wav";

    int ps = 0;
    while( QFile::exists(name) )
    {
        ps++;
        name  = base_name + ".";
        name += QString::number(ps);
        name += ".wav";
    }

    return name;
}

QString BtWavWriter::wordToId(QVector<BtWord> result)
{
    QString buf = "";

    if( result.length()==0 )
    {
        return buf;
    }

    for( int i=0 ; i<result.length()-1 ; i++ )
    {
        for( int j=0 ; j<word_list.length() ; j++ )
        {
            if( result[i].word==word_list[j] )
            {
                buf += QString::number(j);
                buf += "_";

                break;
            }
        }
    }

    QString last_word = result.last().word;
    for( int j=0 ; j<word_list.length() ; j++ )
    {
        if( last_word==word_list[j] )
        {
            buf += QString::number(j);
        }
    }

    return buf;
}
