#include "kd_online.h"

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

typedef kaldi::int32 int32;
typedef kaldi::int64 int64;


void GetDiagnosticsAndPrintOutput(const std::string &utt,
                                  const fst::SymbolTable *word_syms,
                                  const CompactLattice &clat,
                                  int64 *tot_num_frames,
                                  double *tot_like)
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
    KALDI_VLOG(2) << "Likelihood per frame for utterance " << utt << " is "
                << (likelihood / num_frames) << " over " << num_frames
                << " frames.";

    if (word_syms != NULL)
    {
        std::cerr << utt << ' ';
        for( size_t i = 0; i<words.size() ; i++ )
        {
            std::string s = word_syms->Find(words[i]);
            if (s == "")
            {
                KALDI_ERR << "Word-id " << words[i] << " not in symbol table.";
            }
            std::cerr << s << ' ';
        }
        std::cerr << std::endl;
    }
}


void main()
{
    std::string word_syms_rxfilename;

    OnlineEndpointConfig endpoint_config;
    OnlineFeaturePipelineCommandLineConfig feature_cmdline_config;
    OnlineGmmDecodingConfig decode_config;

    BaseFloat chunk_length_secs = 0.05;
    bool do_endpointing = false;

    std::string fst_rxfilename = po.GetArg(1);
    std::string spk2utt_rspecifier = po.GetArg(2);
    std::string wav_rspecifier = po.GetArg(3);
    std::string clat_wspecifier = po.GetArg(4);

    OnlineFeaturePipelineConfig feature_config(feature_cmdline_config);
    OnlineFeaturePipeline pipeline_prototype(feature_config);
    // The following object initializes the models we use in decoding.
    OnlineGmmDecodingModels gmm_models(decode_config);


    fst::Fst<fst::StdArc> *decode_fst = ReadFstKaldiGeneric(fst_rxfilename);

    int32 num_done = 0, num_err = 0;
    double tot_like = 0.0;
    int64 num_frames = 0;

    SequentialTokenVectorReader spk2utt_reader(spk2utt_rspecifier);
    RandomAccessTableReader<WaveHolder> wav_reader(wav_rspecifier);
    CompactLatticeWriter clat_writer(clat_wspecifier);

    OnlineTimingStats timing_stats;

    for (; !spk2utt_reader.Done(); spk2utt_reader.Next())
    {
        const std::vector<std::string> &uttlist = spk2utt_reader.Value();
        OnlineGmmAdaptationState adaptation_state;

        for( size_t i=0 ; i<uttlist.size() ; i++)
        {
            std::string utt = uttlist[i];
            const WaveData &wave_data = wav_reader.Value(utt);
            // get the data for channel zero (if the signal is not mono, we only
            // take the first channel).
            SubVector<BaseFloat> data(wave_data.Data(), 0);

            SingleUtteranceGmmDecoder decoder(decode_config,
                                              gmm_models,
                                              pipeline_prototype,
                                              *decode_fst,
                                              adaptation_state);

            OnlineTimer decoding_timer(utt);

            BaseFloat samp_freq = wave_data.SampFreq();
            int32 chunk_length = int32(samp_freq * chunk_length_secs);
            if (chunk_length == 0)
            {
                chunk_length = 1;
            }

            int32 samp_offset = 0;
            while (samp_offset < data.Dim())
            {
                int32 samp_remaining = data.Dim() - samp_offset;
                int32 num_samp = chunk_length < samp_remaining ? chunk_length
                                                             : samp_remaining;

                SubVector<BaseFloat> wave_part(data, samp_offset, num_samp);
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
            CompactLattice clat;
            bool rescore_if_needed = true;
            decoder.GetLattice(rescore_if_needed, end_of_utterance, &clat);

            GetDiagnosticsAndPrintOutput(utt, word_syms, clat,
                                         &num_frames, &tot_like);

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
            clat_writer.Write(utt, clat);
            KALDI_LOG << "Decoded utterance " << utt;
            num_done++;
        }
    }
    timing_stats.Print();
    KALDI_LOG << "Decoded " << num_done << " utterances, "
              << num_err << " with errors.";
    KALDI_LOG << "Overall likelihood per frame was " << (tot_like / num_frames)
              << " per frame over " << num_frames << " frames.";
    delete decode_fst;
}

KdOnline2::KdOnline2(QObject *parent): QObject(parent)
{
    parseWords(BT_WORDS_PATH);
}

KdOnline2::~KdOnline2()
{
    delete feat_transform;
    delete decode_fst;
    delete decoder;
    delete am_gmm;
    delete trans_model;
}

void KdOnline2::init()
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
    int32 frame_length = mfcc_opts.frame_opts.frame_length_ms = 25;
    int32 frame_shift = mfcc_opts.frame_opts.frame_shift_ms = 10;

    decoder = new OnlineFasterDecoder(*decode_fst, decoder_opts,
                                silence_phones, *trans_model);

    OnlinePaSource au_src(kTimeout, kSampleFreq, kPaRingSize, kPaReportInt);
    Mfcc mfcc(mfcc_opts);
    OnlineFeInput<Mfcc> fe_input(&au_src, &mfcc,
                     frame_length * (kSampleFreq / 1000),
                     frame_shift * (kSampleFreq / 1000));
    OnlineCmnInput cmn_input(&fe_input, cmn_window, min_cmn_window);
    feat_transform = 0;

    DeltaFeaturesOptions opts;
    opts.order = kDeltaOrder;
    feat_transform = new OnlineDeltaInput(opts, &cmn_input);

    startDecode();
}

void KdOnline2::startDecode()
{
    BaseFloat acoustic_scale = 0.05;
    // feature_reading_opts contains number of retries, batch size.
    OnlineFeatureMatrixOptions feature_reading_opts;
    OnlineFeatureMatrix *feature_matrix;
    feature_matrix = new OnlineFeatureMatrix(feature_reading_opts,
                                             feat_transform);

    OnlineDecodableDiagGmmScaled decodable(*am_gmm, *trans_model, acoustic_scale,
                                           feature_matrix);

    decoder->InitDecoding();
    VectorFst<LatticeArc> out_fst;
    clock_t start, end;
    double cpu_time_used;

    while(1)
    {
        start = clock();
        OnlineFasterDecoder::DecodeState dstate = decoder->Decode(&decodable);

        std::vector<int32> word_ids;
        if ( dstate&decoder->kEndUtt )
        {
            decoder->FinishTraceBack(&out_fst);
            fst::GetLinearSymbolSequence(out_fst,
                                         static_cast<vector<int32> *>(0),
                                        &word_ids,
                                         static_cast<LatticeArc::Weight*>(0));
            if( word_ids.size() )
            {
                execute(word_ids, &history);
                writeBarResult();
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                qDebug() << cpu_time_used;
            }
            system("dbus-send --session --dest=com.binaee.rebound / "
                               "com.binaee.rebound.exec  string:\"\"");
        }
        else
        {
            if (decoder->PartialTraceback(&out_fst))
            {
                fst::GetLinearSymbolSequence(out_fst,
                                           static_cast<vector<int32> *>(0),
                                           &word_ids,
                                           static_cast<LatticeArc::Weight*>(0));
                execute(word_ids, &history);
                writeBarResult();
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                qDebug() << cpu_time_used;
            }
            else
            {
                system("dbus-send --session --dest=com.binaee.rebound / "
                               "com.binaee.rebound.exec  string:\"\"");
            }
        }
    }
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
