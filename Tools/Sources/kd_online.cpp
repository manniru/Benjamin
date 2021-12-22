#include "kd_online.h"
#include "online/onlinebin-util.h"
#include "online/online-decodable.h"
#include "online/online-faster-decoder.h"
// calc conf

using namespace kaldi;
using namespace fst;

typedef kaldi::int32 int32;
typedef OnlineDecodableDiagGmmScaled KdGmmDecodable;

fst::Fst<fst::StdArc> *decode_fst;
AmDiagGmm             *am_gmm;
TransitionModel       *trans_model;
BT_ONLINE_DECODER     *o_decoder;
KdOnline2Model        *o2_model; //gaussain online 2 model

KdOnline::KdOnline(QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);
    parseWords(BT_WORDS_PATH);

    ab_src = new BtRecorder(cy_buf);
}

KdOnline::~KdOnline()
{
    delete decode_fst;
    delete o_decoder;
    delete am_gmm;
    delete trans_model;
}

void KdOnline::init()
{
    BT_ONLINE_OPTS decoder_opts;

    std::string model_rxfilename = BT_MDL_PATH;
    std::string fst_rxfilename   = BT_FST_PATH;
    QVector<int> silence_phones = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    trans_model = new TransitionModel;
    am_gmm = new AmDiagGmm;

    bool rx_binary;
    Input ki(model_rxfilename, &rx_binary);
    trans_model->Read(ki.Stream(), rx_binary);
    am_gmm->Read(ki.Stream(), rx_binary);

    decode_fst = ReadDecodeGraph(fst_rxfilename);

    decoder_opts.max_active = 7000;
    decoder_opts.beam = 13.0;

    OnlineGmmDecodingConfig d_config;
    d_config.fmllr_basis_rxfilename = KAL_NATO_DIR"exp/tri1_online/fmllr.basis";
    d_config.online_alimdl_rxfilename = KAL_NATO_DIR"exp/tri1_online/final.oalimdl";
    o2_model = new KdOnline2Model(trans_model, am_gmm, &d_config);

#ifdef BT_LAT_ONLINE
    decoder_opts.lattice_beam = 6.0;
#else

#endif

    o_decoder = new BT_ONLINE_DECODER(*decode_fst, decoder_opts,
                                silence_phones, *trans_model);


    startDecode();
}

void KdOnline::startDecode()
{
    float acoustic_scale = 0.08;

    BT_ONLINE_DECODABLE decodable(ab_src, o2_model,
                             acoustic_scale);

    ab_src->startStream();
#ifdef BT_LAT_ONLINE
    KdOnline2Decodable decodable(ab_src,
                                 o2_model, acoustic_scale);
#endif

    o_decoder->InitDecoding();
    BT_ONLINE_LAT out_fst;
    int tok_count;

    clock_t start;

    while(1)
    {
        start = clock();

#ifdef BT_LAT_ONLINE
        decodable.features->AcceptWaveform(cy_buf);
#endif
        int dstate = o_decoder->Decode(&decodable);

        if( dstate==KD_STATE_SILENCE )
        {
            tok_count = o_decoder->FinishTraceBack(&out_fst);
            processLat(&out_fst, start);

            if( 1 )
            {
                system("dbus-send --session --dest=com.binaee.rebound / "
                       "com.binaee.rebound.exec  string:\"\"");
            }
        }
        else
        {
            tok_count = o_decoder->PartialTraceback(&out_fst);
            if( tok_count )
            {
                processLat(&out_fst, start);
            }
            else if( BT_IMMDT_EXEC )
            {
                system("dbus-send --session --dest=com.binaee.rebound / "
                               "com.binaee.rebound.exec  string:\"\"");
            }
        }
    }
}

void KdOnline::printTime(clock_t start)
{
    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    qDebug() << cpu_time_used;
}

void KdOnline::execute(std::vector<int32_t> word)
{
    QString cmd = KAL_SI_DIR"main.sh \"";

#ifdef BT_LAT_ONLINE
    history.clear();
#endif
    for( int i=0 ; i<word.size() ; i++ )
    {
        QString word_str = lexicon[word[i]];
        cmd += word_str;
        cmd += " ";
        history.push_back(word_str);

#ifndef BT_LAT_ONLINE
        if( history.size()>10 )
        {
            history.pop_front();
        }
#endif
    }
    cmd += "\"";
    system(cmd.toStdString().c_str());
}

void KdOnline::processLat(BT_ONLINE_LAT *clat, clock_t start)
{
    if( clat->Start() )
    {
        return;
    }
    QVector<BtWord> result;

    KdMBR *mbr = NULL;
    mbr = new KdMBR(clat);
    result = mbr->getResult(lexicon);

#ifndef BT_LAT_ONLINE
    for( int i=0 ; i<result.size() ; i++ )
    {
        qDebug() << result[i].word << result[i].conf;// << conf[i];
    }
#endif
    bt_writeBarResult(result);
}

void KdOnline::parseWords(QString filename)
{
    QFile words_file(filename);

    if (!words_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << filename;
        return;
    }

    lexicon.clear();

    while (!words_file.atEnd())
    {
        QString line = QString(words_file.readLine());
        QStringList line_list = line.split(" ");
        lexicon.append(line_list[0]);
    }

    words_file.close();
}
