#include "kd_online.h"

KdOnline::KdOnline(BtState *state, QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);

    ab_src = new BtRecorder(cy_buf, state);
    wav_w  = new BtWavWriter(cy_buf, state);

    oa_model = new KdAModel;
    t_model = new KdTransitionModel;

    std::ifstream ki;
    ki.open(state->mdl_path, std::ios_base::in |
                             std::ios_base::binary);
    ki.get();
    ki.get();
    t_model->Read(ki);
    oa_model->Read(ki);

    o_decoder = new KdOnlineLDecoder(t_model, state);
    cap = new BtCaptain(state);
    st  = state;
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
    KdDecodable decodable(cy_buf, oa_model,
                          t_model, st);
    cap->net->cfb   = decodable.features->o_features;
    decodable.features->enableENN();

    ab_src->startStream();

    o_decoder->InitDecoding(&decodable);
    KdCompactLattice out_fst;

    while( 1 )
    {
        if( isHalt() )
        {
            cy_buf->clear();
            QThread::msleep(200);
            continue;
        }
        decodable.features->ComputeFeatures();
        o_decoder->Decode();
        c_result = o_decoder->getResult(&out_fst);
        processResult(c_result);

        //if finished
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
        result[i].stf   = 100*result[i].start +
                          o_decoder->status.min_frame;
        buf += result[i];
    }
    cap->parse(buf, o_decoder->frame_num);
}

void KdOnline::writeWav(int len)
{
    if( isSleep() )
    {
        return; //dont record in sleep
    }
    qDebug() << "data len" << o_decoder->uframe;

    if(  o_decoder->wav_id<BT_WAV_MAX )
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
#ifdef WIN32
    QString path = MOM_LABEL_DIR;
    path += MOM_LABEL_STATUS;

    if( QFile::exists(path) )
    {
        QString status = readStatusFile();
        if( status.contains("Sleep") )
        {
            return true;
        }
    }
    return false;
#else
    QString status_path = getenv("HOME");
    status_path += "/.config/polybar/awesomewm/ben_status";
    if( QFile::exists(status_path) )
    {
        QString cmd = "cat ";
        cmd += status_path;
        QString status = getStrCommand(cmd);

        if( status.contains("Sleep", Qt::CaseInsensitive) )
        {
            return true;
        }
    }
    return false;
#endif
}

bool KdOnline::isHalt()
{
#ifdef WIN32
    int s_state = GetKeyState(VK_SCROLL);
    QString path = MOM_LABEL_DIR;
    path += MOM_LABEL_STATUS;

    if( s_state )
    {
        QString status = readStatusFile();
        if( !status.contains("Halt") )
        {
            writeHalt(path);
        }

        return true;
    }
    else if( QFile::exists(path) )
    {
        QString status = readStatusFile();
        if( status.contains("Halt") )
        {
            QString cmd = "del ";
            cmd += path;

            system(cmd.toStdString().c_str());
        }
    }
#else
    QString status_path = getenv("HOME");
    status_path += "/.config/polybar/awesomewm/ben_status";
    if( QFile::exists(status_path) )
    {
        QString cmd = "cat ";
        cmd += status_path;
        QString status = getStrCommand(cmd);

        if( status.contains("halt", Qt::CaseInsensitive) )
        {
            return true;
        }
        return false;
    }
#endif
    return false;
}

void KdOnline::writeHalt(QString path)
{
    QFile st_file(path);

    if( !st_file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        qDebug() << "Error creating" << MOM_LABEL_STATUS;
        qDebug() << "Trying to create" << MOM_LABEL_DIR;
        system("mkdir " MOM_LABEL_DIR);
    }
    QTextStream out(&st_file);
    out << "  %{B#666666}%{F#ffffff}%{A1:$HS_CMD:}"
        << "  Halt  "
        << "%{A1}%{B- F1-}";
    st_file.close();
}

QString KdOnline::readStatusFile()
{
    QString path = MOM_LABEL_DIR;
    path += MOM_LABEL_STATUS;
    QFile file(path);
    if( file.open(QIODevice::ReadOnly) )
    {
        QString line = file.readLine();
        line.replace('\n', "");
        file.close();

        return line;
    }

    return "";
}
