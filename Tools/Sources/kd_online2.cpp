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


void KdOnline2::print(int64 *tot_num_frames, double *tot_like)
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
    *tot_num_frames += num_frames;
    *tot_like += likelihood;
    KALDI_VLOG(2) << "Likelihood per frame for utterance " << " is "
                << (likelihood / num_frames) << " over " << num_frames
                << " frames.";

    QString message;
    for( size_t i = 0; i<words.size() ; i++ )
    {
        message += lexicon[words[i]];
    }
    message +=  ' ';
    qDebug() << message;
}


void KdOnline2::processData(float *wav_data, int len)
{
    OnlineEndpointConfig endpoint_config;
    OnlineFeaturePipelineCommandLineConfig fcc; //feature_cmdline_config
    OnlineGmmDecodingConfig decode_config;
    int counter = 0;

    BaseFloat chunk_length_secs = 0.05;
    bool do_endpointing = false;

    decode_config.silence_phones = "1:2:3:4:5:6:7:8:9:10";
    decode_config.model_rxfilename = BT_FINAL_PATH;
    decode_config.acoustic_scale = 0.05;
    decode_config.fmllr_basis_rxfilename = KAL_NATO_DIR"exp/tri1_online/fmllr.basis";
    decode_config.online_alimdl_rxfilename = KAL_NATO_DIR"exp/tri1_online/final.oalimdl";
    decode_config.faster_decoder_opts.max_active = 7000;
    decode_config.faster_decoder_opts.beam = 12.0;
    decode_config.faster_decoder_opts.lattice_beam = 6.0;

    fcc.add_deltas = true;
    fcc.cmvn_config = KAL_NATO_DIR"exp/tri1_online/conf/online_cmvn.conf";
    fcc.global_cmvn_stats_rxfilename = KAL_NATO_DIR"exp/tri1_online/global_cmvn.stats";
    fcc.mfcc_config = KAL_NATO_DIR"exp/tri1_online/conf/mfcc.conf";

    endpoint_config.silence_phones = "1:2:3:4:5:6:7:8:9:10";

    std::string fst_rxfilename = BT_FST_PATH;
    std::string clat_wspecifier = "ark:"KAL_NATO_DIR"online/lattice.ark";

    OnlineFeaturePipelineConfig feature_config(fcc);
    OnlineFeaturePipeline pipeline_prototype(feature_config);
    // The following object initializes the models we use in decoding.
    OnlineGmmDecodingModels gmm_models(decode_config);


    fst::Fst<fst::StdArc> *decode_fst = ReadFstKaldiGeneric(fst_rxfilename);

    double tot_like = 0.0;
    int64 num_frames = 0;

    CompactLatticeWriter clat_writer(clat_wspecifier);

    OnlineTimingStats timing_stats;

    OnlineGmmAdaptationState adaptation_state;

    // get the data for channel zero (if the signal is not mono, we only
    // take the first channel).
    SubVector<float> data(wav_data, len);

    SingleUtteranceGmmDecoder decoder(decode_config,
                                      gmm_models,
                                      pipeline_prototype,
                                      *decode_fst,
                                      adaptation_state);

    OnlineTimer decoding_timer("1");

    BaseFloat samp_freq = 16000; ///FIXME
    int32 chunk_length = int32(samp_freq * chunk_length_secs);
    if( chunk_length==0 )
    {
        chunk_length = 1;
    }

    int32 samp_offset = 0;
    while (samp_offset < data.Dim())
    {
        int32 samp_remaining = data.Dim() - samp_offset;
        int32 num_samp = chunk_length < samp_remaining ? chunk_length
                                                       : samp_remaining;

        SubVector<float> wave_part(data, samp_offset, num_samp);
        decoder.FeaturePipeline().AcceptWaveform(samp_freq, wave_part);

        samp_offset += num_samp;
        decoding_timer.WaitUntil(samp_offset / samp_freq);
        if (samp_offset == data.Dim())
        {
            // no more input. flush out last frames
            decoder.FeaturePipeline().InputFinished();
        }
        decoder.AdvanceDecoding();

        if (do_endpointing && decoder.EndpointDetected(endpoint_config))
        {
            break;
        }
    }
    decoder.FinalizeDecoding();

    bool end_of_utterance = true;
    decoder.EstimateFmllr(end_of_utterance);
    bool rescore_if_needed = true;
    decoder.GetLattice(rescore_if_needed, end_of_utterance, &clat);

    print(&num_frames, &tot_like);

    decoding_timer.OutputStats(&timing_stats);

    // In an application you might avoid updating the adaptation state if
    // you felt the utterance had low confidence.  See lat/confidence.h
    decoder.GetAdaptationState(&adaptation_state);

    // we want to output the lattice with un-scaled acoustics.
    if (decode_config.acoustic_scale != 0.0)
    {
        BaseFloat inv_acoustic_scale = 1.0 / decode_config.acoustic_scale;
        ScaleLattice(AcousticLatticeScale(inv_acoustic_scale), &clat);
    }
    clat_writer.Write("1", clat); //utt=1
    qDebug() << "Decoded utterance ";
}

KdOnline2::KdOnline2(QObject *parent): QObject(parent)
{
    parseWords(BT_WORDS_PATH);
}

KdOnline2::~KdOnline2()
{
}

void KdOnline2::init()
{

}

void KdOnline2::startDecode()
{

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
