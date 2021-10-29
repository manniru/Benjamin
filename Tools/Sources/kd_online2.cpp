#include "kd_online2.h"

#ifdef BT_ONLINE2
#include "feat/wave-reader.h"
#include "online2/online-feature-pipeline.h"
#include "online2/online-gmm-decoding.h"
#include "online2/onlinebin-util.h"
#include "online2/online-timing.h"
#include "online2/online-endpoint.h"
#include "fstext/fstext-lib.h"
#include "lat/lattice-functions.h"
#include "lat/sausages.h" //MBR

using namespace kaldi;
using namespace fst;

KdOnline2::KdOnline2(QObject *parent): QObject(parent)
{
    cy_buf = new BtCyclic(BT_REC_RATE*BT_BUF_SIZE);
    parseWords(BT_WORDS_PATH);

    g_decoder = new KdOnline2Gmm(parent);
    rec_src = new BtOnlineSource(cy_buf);
}

KdOnline2::~KdOnline2()
{

}

void KdOnline2::init()
{
    int sample_count = BT_REC_SIZE*BT_REC_RATE;
    int16_t raw[BT_REC_SIZE*BT_REC_RATE];
    float raw_f[BT_REC_SIZE*BT_REC_RATE];

    rec_src->startStream();

    while( cy_buf->getDataSize()>BT_REC_SIZE*BT_REC_RATE )
    {
        QThread::sleep(2);
    }
    cy_buf->read(raw, (BT_REC_SIZE-BT_DEC_TIMEOUT)*BT_REC_RATE);

    while( true )
    {
        if( cy_buf->getDataSize()<BT_DEC_TIMEOUT*BT_REC_RATE )
        {
            QThread::sleep(2);
            continue;
        }

        cy_buf->rewind((BT_REC_SIZE-BT_DEC_TIMEOUT)*BT_REC_RATE);
        cy_buf->read(raw, sample_count);

        for( int i=0 ; i<sample_count ; i++ )
        {
            raw_f[i] = raw[i];
        }
        processData(raw_f, sample_count);
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

void KdOnline2::print(CompactLattice *clat)
{
    MinimumBayesRiskOptions mbr_opts;
    MinimumBayesRisk *mbr = NULL;
    mbr_opts.decode_mbr = true;

    mbr = new MinimumBayesRisk(*clat, mbr_opts);
    vector<float> conf = mbr->GetOneBestConfidences();
    const vector<int32> &words = mbr->GetOneBest();
    const vector<pair<float, float>> &times = mbr->GetOneBestTimes();

    QString message;
    for( int i = 0; i<words.size() ; i++ )
    {
        float time_c = (times[i].first+times[i].second)/2/100;
        message += lexicon[words[i]];
        message +=  "[";
        message += QString::number(conf[i]);
        message +=  ", ";
        message += QString::number(time_c,'g', 2);
        message +=  "] ";
    }

    if( words.size()>10 )
    {
        CompactLatticeWriter clat_writer("ark:b.ark");
        clat_writer.Write("f", *clat);
        exit(0);
    }

    qDebug() << "print" << message;
}

void KdOnline2::processData(float *wav_data, int len)
{
    clock_t start = clock();
    g_decoder->init();
    printTime(start);
    BaseFloat chunk_length_secs = 0.05;

    // get the data for channel zero (if the signal is not mono, we only
    // take the first channel).
    SubVector<float> data(wav_data, len);

    BaseFloat samp_freq = 16000; ///FIXME
    int32 chunk_length = int32(samp_freq * chunk_length_secs);

    int32 samp_offset = 0;
//    qDebug() << "data.Dim()" << data.Dim() << "len" << len;
    while( samp_offset<data.Dim() )
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
        g_decoder->feature_pipeline->AcceptWaveform(samp_freq, wave_part);

        samp_offset += num_samp;
//        if (samp_offset == data.Dim())
//        {
//            // no more input. flush out last frames
//            g_decoder->FeaturePipeline().InputFinished();
//        }
        g_decoder->AdvanceDecoding();

    }
    g_decoder->FinalizeDecoding();

    CompactLattice clat;
    g_decoder->GetLattice(true, true, &clat);

    print(&clat);

    // In an application you might avoid updating the adaptation state if
    // you felt the utterance had low confidence.  See lat/confidence.h
//    g_decoder->GetAdaptationState(&adaptation_state);
}

void KdOnline2::printTime(clock_t start)
{
    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    qDebug() << "cpu time" << qRound(cpu_time_used*1000) << "ms";
}

#endif
