#include "kd_online.h"

KdOnline::KdOnline(QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);
    cap = new BtCaptain;

    ab_src = new BtRecorder(cy_buf);
    wav_w  = new BtWavWriter(cy_buf);

    std::string model_filename = BT_OAMDL_PATH;

    oa_model = new KdAModel;
    t_model = new kaldi::TransitionModel;

    bool binary;
    kaldi::Input ki(model_filename, &binary);
    t_model->Read(ki.Stream(), binary);
    oa_model->Read(ki.Stream(), binary);

    o_decoder = new KdOnlineLDecoder(t_model);
}

KdOnline::~KdOnline()
{
    delete t_model;
    delete oa_model;
    delete o_decoder;
}

void KdOnline::init()
{
    startDecode();
}

void KdOnline::startDecode()
{
    float acoustic_scale = 0.05;

    KdDecodable decodable(cy_buf, oa_model,
                          t_model, acoustic_scale);

    ab_src->startStream();

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;

    while( 1 )
    {
        decodable.features->ComputeFeatures();
        o_decoder->Decode();
        c_result = o_decoder->getResult(&out_fst);
        processResult(c_result);

        if( o_decoder->status.state!=KD_STATE_NORMAL )
        {
            cap->flush();
            if( c_result.length() )
            {
                writeWav(o_decoder->uframe*160);
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
                qDebug() << "skipped end" << result[i].word
                         << result[i].end;
                continue;
            }
        }
        result[i].time += o_decoder->status.min_frame/100.0;
        buf += result[i];
    }
    cap->parse(buf);
}

void KdOnline::writeWav(int len)
{
    if( isSleep() )
    {
        return; //dont record in sleep
    }
    qDebug() << "data len" << o_decoder->uframe;

    if ( o_decoder->wav_id<BT_WAV_MAX )
    {
        o_decoder->wav_id++;
    }
    else
    {
        o_decoder->wav_id = 0;
    }
    wav_w->write(c_result, len, o_decoder->wav_id);

}

bool KdOnline::isSleep()
{
    QString status_path = getenv("HOME");
    status_path += "/.config/polybar/awesomewm/ben_status";
    if( QFile::exists(status_path) )
    {
        return true;
    }
    return false;
}
