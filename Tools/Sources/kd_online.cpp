#include "kd_online.h"

using namespace kaldi;
using namespace fst;

KdOnline::KdOnline(QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);
    cap = new BtCaptain;

    ab_src = new BtRecorder(cy_buf);

    std::string model_rxfilename = BT_OAMDL_PATH;

    o2_model = new KdModel(model_rxfilename);
    o_decoder = new KdOnlineLDecoder(*(o2_model->t_model));
}

KdOnline::~KdOnline()
{
    delete o2_model;
    delete o_decoder;
}

void KdOnline::init()
{
    startDecode();
}

void KdOnline::startDecode()
{
    float acoustic_scale = 0.05;

    KdDecodable decodable(ab_src, o2_model,
                             acoustic_scale);

    ab_src->startStream();

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;
    QVector<BtWord> result;

    while( 1 )
    {
        decodable.features->AcceptWaveform(cy_buf);
        o_decoder->Decode();
        result = o_decoder->getResult(&out_fst);
        processResult(result);

        if( o_decoder->status.state!=KD_STATE_NORMAL )
        {
            cap->flush();
            if( result.length() )
            {
                writeWav(cy_buf, o_decoder->uframe*160);
            }
        }
    }
}

void KdOnline::processResult(QVector<BtWord> result)
{
    if( result.empty() )
    {
        return;
    }

    QVector<BtWord> buf;

    for( int i=0 ; i<result.size() ; i++ )
    {
        if( i==0 )
        {
            if( result[i].conf<0.75 &&
                result.size()<3 && //this is realy hard? no!
                o_decoder->status.state==KD_STATE_NORMAL)
            {
                continue;
            }
            if( result[i].end<0.15 )
            {
                continue;
            }
        }
        result[i].time += o_decoder->status.min_frame/100.0;
        buf += result[i];
    }
    cap->parse(buf);
}

void KdOnline::writeWav(BtCyclic *buf, int len)
{
    QString status_path = getenv("HOME");
    status_path += "/.config/polybar/awesomewm/ben_status";
    if( QFile::exists(status_path) )
    {
        return; //sleep mode
    }
    int16_t *data = (int16_t *)malloc(len*sizeof(int16_t));
    buf->rewind(len);
    buf->read(data, len);

    if ( o_decoder->wav_id<BT_WAV_MAX )
    {
        o_decoder->wav_id++;
    }
    else
    {
        o_decoder->wav_id = 0;
    }
    QString filename = KAL_AU_DIR"/online/";
    filename += QString::number(o_decoder->wav_id);
    filename += ".wav";
    QFile *file = new QFile(filename);

    if( !file->open(QIODevice::WriteOnly) )
    {
        qDebug() << "Failed To Open" << filename;
        exit(1);
    }

    writeWavHeader(file, len);
    for( int i=0 ; i<len ; i++ )
    {
        int16_t *pt = &data[i];
        // kaldi should be 2 channel
        file->write((char *)pt, 2);
        file->write((char *)pt, 2);
    }
    file->close();
}

void KdOnline::writeWavHeader(QFile *file, int len)
{
    char buff[200];
    int32_t buf_i;
    int16_t buf_s; //short

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
    buf_i = len; file->write(buff,4);//subchunk2 size(int=sample count)
    qDebug() << "sample_rate:"  << buf_i;
}
