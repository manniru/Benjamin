#include "kd_online.h"
#include "feat/feature-mfcc.h"
#include "online/online-feat-input.h"
#include "online/online-decodable.h"
#include "online/online-faster-decoder.h"
#include "online/onlinebin-util.h"
// calc conf

using namespace kaldi;
using namespace fst;

typedef kaldi::int32 int32;
typedef OnlineDecodableDiagGmmScaled KdGmmDecodable;
typedef OnlineFasterDecoder::DecodeState KdDState;

OnlineFeatInputItf    *feat_transform;
fst::Fst<fst::StdArc> *decode_fst;
OnlineFasterDecoder   *decoder;
AmDiagGmm             *am_gmm;
TransitionModel       *trans_model;

KdOnline::KdOnline(BtCyclic *buffer, QObject *parent): QObject(parent)
{
    parseWords(BT_WORDS_PATH);

    ab_src = new BtOnlineSource(buffer);
}

KdOnline::~KdOnline()
{
    delete feat_transform;
    delete decode_fst;
    delete decoder;
    delete am_gmm;
    delete trans_model;
}

void KdOnline::init()
{
    OnlineFasterDecoderOpts decoder_opts;

    std::string model_rxfilename = BT_FINAL_PATH;
    std::string fst_rxfilename   = BT_FST_PATH;
    std::string silence_phones_str = "1:2:3:4:5:6:7:8:9:10";

    std::vector<int32> silence_phones;
    SplitStringToIntegers(silence_phones_str, ":", false, &silence_phones);

    trans_model = new TransitionModel;
    am_gmm = new AmDiagGmm;

    bool rx_binary;
    Input ki(model_rxfilename, &rx_binary);
    trans_model->Read(ki.Stream(), rx_binary);
    am_gmm->Read(ki.Stream(), rx_binary);

    decode_fst = ReadDecodeGraph(fst_rxfilename);

    // We are not properly registering/exposing MFCC and frame extraction options,
    // because there are parts of the online decoding code, where some of these
    // options are hardwired(ToDo: we should fix this at some point)
    MfccOptions mfcc_opts;
    mfcc_opts.use_energy = false;
    int32 frame_length = mfcc_opts.frame_opts.frame_length_ms = 20;
    int32 frame_shift = mfcc_opts.frame_opts.frame_shift_ms = 5;

    decoder_opts.max_active = 7000;
    decoder_opts.beam = 15.0;
    decoder = new OnlineFasterDecoder(*decode_fst, decoder_opts,
                                silence_phones, *trans_model);

//    OnlinePaSource au_src(500, kSampleFreq, 32768, kPaReportInt);
    Mfcc mfcc(mfcc_opts);
    OnlineFeInput<Mfcc> fe_input(ab_src, &mfcc,
                     frame_length * (kSampleFreq / 1000),
                     frame_shift * (kSampleFreq / 1000));
    OnlineCmnInput cmn_input(&fe_input, cmn_window, min_cmn_window);
    feat_transform = 0;

    DeltaFeaturesOptions opts;
    opts.order = kDeltaOrder;
    feat_transform = new OnlineDeltaInput(opts, &cmn_input);

    startDecode();
}

void KdOnline::startDecode()
{
    BaseFloat acoustic_scale = 0.08;
    // feature_reading_opts contains number of retries, batch size.
    OnlineFeatureMatrixOptions feature_reading_opts;
    OnlineFeatureMatrix *feature_matrix;
    feature_matrix = new OnlineFeatureMatrix(feature_reading_opts,
                                             feat_transform);

    KdGmmDecodable decodable(*am_gmm, *trans_model,
                             acoustic_scale, feature_matrix);

    decoder->InitDecoding();
    VectorFst<LatticeArc> out_fst;

    clock_t start;

    while(1)
    {
        start = clock();
        KdDState dstate = decoder->Decode(&decodable);

        if( dstate&decoder->kEndUtt )
        {
            decoder->FinishTraceBack(&out_fst);
            processLat(&out_fst, start);

            system("dbus-send --session --dest=com.binaee.rebound / "
                               "com.binaee.rebound.exec  string:\"\"");
        }
        else
        {
            if( decoder->PartialTraceback(&out_fst) )
            {
                processLat(&out_fst, start);
            }
            else
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
    for( int i=0 ; i<word.size() ; i++ )
    {
        QString word_str = lexicon[word[i]];
        cmd += word_str;
        cmd += " ";
        history.push_back(word_str);

        if( history.size()>10 )
        {
            history.pop_front();
        }
    }
    cmd += "\"";
    system(cmd.toStdString().c_str());
}

void KdOnline::processLat(VectorFst<LatticeArc> *fst_in, clock_t start)
{
    MinimumBayesRiskOptions mbr_opts;
    MinimumBayesRisk *mbr = NULL;
    mbr_opts.decode_mbr = true;

    Lattice lat;
    CompactLattice clat;

    vector<int32> word_ids;

    vector<int32> *isymbols_out = NULL;
    LatticeArc::Weight *w_out = NULL;

    GetLinearSymbolSequence(*fst_in, isymbols_out,
                            &word_ids, w_out);
    ConvertLattice(*fst_in, &lat);
    ConvertLattice(lat, &clat);
    ScaleLattice(LatticeScale(1.0, 0.00001), &clat);

    mbr = new MinimumBayesRisk(clat, mbr_opts);

    vector<float> conf = mbr->GetOneBestConfidences();

    for( int i=0 ; i<conf.size() ; i++ )
    {
        qDebug() << lexicon[word_ids[i]] << conf[i];

        if( i>1 )
        {
            CompactLatticeWriter clat_writer("ark,t:b.ark");
            clat_writer.Write("f", clat);
//            TableWriter<fst::VectorFstHolder> fst_writer("ark:1.fsts");
//            fst_writer.Write("f", *fst_in);

            exit(0);
        }
    }

    if( word_ids.size() )
    {
        execute(word_ids);
//        printTime(start);
        writeBarResult();
    }
}

void KdOnline::writeBarResult()
{
    QFile bar_file(BT_BAR_RESULT);

    if (!bar_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Error opening" << BT_BAR_RESULT;
        return;
    }

    QTextStream out(&bar_file);

    for( int i=0 ; i<history.length() ; i++ )
    {
        out << "%{u#1d1}%{+u}";
        out << history[i];

        out << "%{-u} ";
    }
    out << "\n";

    bar_file.close();
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
