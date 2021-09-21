#include "kd_online2.h"

#include "feat/wave-reader.h"
#include "online2/online-feature-pipeline.h"
#include "online2/online-gmm-decoding.h"
#include "online2/onlinebin-util.h"
#include "online2/online-timing.h"
#include "online2/online-endpoint.h"
#include "fstext/fstext-lib.h"
#include "lat/lattice-functions.h"

using namespace kaldi;
using namespace fst;

CompactLattice clat;
SingleUtteranceGmmDecoder *g_decoder;
OnlineGmmAdaptationState adaptation_state;


OnlineFeaturePipeline *pipeline_prototype;
// The following object initializes the models we use in decoding.
OnlineGmmDecodingModels *gmm_models;

void KdOnline2::print()
{
    if (clat.NumStates() == 0)
    {
        KALDI_WARN << "Empty lattice.";
        return;
    }
    CompactLattice best_path_clat;
    CompactLatticeShortestPath(clat, &best_path_clat);

    Lattice best_path_lat;
    ConvertLattice(best_path_clat, &best_path_lat);

    double likelihood;
    LatticeWeight weight;
    int32 num_frames;
    std::vector<int32> alignment;
    std::vector<int32> words;
    GetLinearSymbolSequence(best_path_lat, &alignment, &words, &weight);
    num_frames = alignment.size();
    likelihood = -(weight.Value1() + weight.Value2());
    qDebug() << "Likelihood per frame for utterance " << " is "
                << (likelihood ) << " over " << num_frames
                << " frames.";

    QString message;
    for( size_t i = 0; i<words.size() ; i++ )
    {
        message += lexicon[words[i]];
        message +=  ' ';
    }
    qDebug() << message;
}

void KdOnline2::processData(float *wav_data, int len)
{
    BaseFloat chunk_length_secs = 0.05;

    // get the data for channel zero (if the signal is not mono, we only
    // take the first channel).
    SubVector<float> data(wav_data, len);

    BaseFloat samp_freq = 16000; ///FIXME
    int32 chunk_length = int32(samp_freq * chunk_length_secs);

    int32 samp_offset = 0;
//    qDebug() << "data.Dim()" << data.Dim() << "len" << len;
    while (samp_offset < data.Dim())
    {
        int32 samp_remaining = data.Dim() - samp_offset;

        int32 num_samp;
        if( chunk_length<samp_remaining )
        {
            num_samp = chunk_length;
        }
        else
        {
            num_samp = samp_remaining;
        }

        SubVector<float> wave_part(data, samp_offset, num_samp);
        g_decoder->FeaturePipeline().AcceptWaveform(samp_freq, wave_part);

        samp_offset += num_samp;
//        if (samp_offset == data.Dim())
//        {
//            // no more input. flush out last frames
//            g_decoder->FeaturePipeline().InputFinished();
//        }
        g_decoder->AdvanceDecoding();

    }
    g_decoder->FinalizeDecoding();

    bool end_of_utterance = true;
//    g_decoder->EstimateFmllr(end_of_utterance);
    bool rescore_if_needed = true;
    g_decoder->GetLattice(rescore_if_needed, end_of_utterance, &clat);

    print();

    // In an application you might avoid updating the adaptation state if
    // you felt the utterance had low confidence.  See lat/confidence.h
//    g_decoder->GetAdaptationState(&adaptation_state);
}

KdOnline2::KdOnline2(QObject *parent): QObject(parent)
{
    parseWords(BT_WORDS_PATH);

    init();
}

KdOnline2::~KdOnline2()
{
}

void KdOnline2::init()
{
    OnlineEndpointConfig endpoint_config;
    OnlineFeaturePipelineCommandLineConfig fcc; //feature_cmdline_config
    OnlineGmmDecodingConfig decode_config;

    fcc.mfcc_config = KAL_NATO_DIR"exp/tri1_online/conf/mfcc.conf";
    fcc.cmvn_config = KAL_NATO_DIR"exp/tri1_online/conf/online_cmvn.conf";
    fcc.add_deltas = true;
    fcc.global_cmvn_stats_rxfilename = KAL_NATO_DIR"exp/tri1_online/global_cmvn.stats";

    decode_config.fmllr_basis_rxfilename = KAL_NATO_DIR"exp/tri1_online/fmllr.basis";
    decode_config.online_alimdl_rxfilename = KAL_NATO_DIR"exp/tri1_online/final.oalimdl";
    decode_config.model_rxfilename = KAL_NATO_DIR"exp/"KAL_MODE"/final.mdl";
    decode_config.silence_phones = "1:2:3:4:5:6:7:8:9:10";
    decode_config.faster_decoder_opts.max_active = 7000;
    decode_config.faster_decoder_opts.beam = 12.0;
    decode_config.faster_decoder_opts.lattice_beam = 6.0;
    decode_config.acoustic_scale = acoustic_scale;

    endpoint_config.silence_phones = "1:2:3:4:5:6:7:8:9:10";

    std::string fst_rxfilename = KAL_NATO_DIR"exp/"KAL_MODE"/graph/HCLG.fst";


    OnlineFeaturePipelineConfig feature_config(fcc);
    pipeline_prototype = new OnlineFeaturePipeline(feature_config);
    // The following object initializes the models we use in decoding.
    gmm_models = new OnlineGmmDecodingModels(decode_config);

    Fst<StdArc> *decode_fst2 = ReadFstKaldiGeneric(fst_rxfilename);

    g_decoder = new SingleUtteranceGmmDecoder(decode_config,
                                              *gmm_models,
                                              *pipeline_prototype,
                                              *decode_fst2,
                                              adaptation_state);
}

void KdOnline2::startDecode()
{
    double tot_like = 0.0;
    int64 num_frames = 0;

    OnlineTimingStats timing_stats;
}

void KdOnline2::execute(std::vector<int32_t> word, QVector<QString> *history)
{
    QString cmd = KAL_SI_DIR"main.sh \"";
    for( int i=0 ; i<word.size() ; i++ )
    {
        QString word_str = lexicon[word[i]];
        cmd += word_str;
        cmd += " ";
        history->push_back(word_str);

        if( history->size()>10 )
        {
            history->pop_front();
        }
    }
    cmd += "\"";
    system(cmd.toStdString().c_str());
}

void KdOnline2::writeBarResult()
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

void KdOnline2::parseWords(QString filename)
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
