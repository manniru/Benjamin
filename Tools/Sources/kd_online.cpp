#include "kd_online.h"
#include "feat/feature-mfcc.h"
#include "online/online-audio-source.h"
#include "online/online-feat-input.h"
#include "online/online-decodable.h"
#include "online/online-faster-decoder.h"
#include "online/onlinebin-util.h"

using namespace kaldi;
using namespace fst;

typedef kaldi::int32 int32;

OnlineFeatInputItf    *feat_transform;
fst::SymbolTable      *word_syms;
fst::Fst<fst::StdArc> *decode_fst;
OnlineFasterDecoder   *decoder;
AmDiagGmm             *am_gmm;
TransitionModel       *trans_model;

KdOnline::KdOnline(QObject *parent): QObject(parent)
{

}

KdOnline::~KdOnline()
{
    delete feat_transform;
    delete word_syms;
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
    std::string word_syms_filename = BT_WORDS_PATH;
    std::string silence_phones_str = "1:2:3:4:5:6:7:8:9:10";

    std::vector<int32> silence_phones;
    SplitStringToIntegers(silence_phones_str, ":", false, &silence_phones);

    trans_model = new TransitionModel;
    am_gmm = new AmDiagGmm;

    bool rx_binary;
    Input ki(model_rxfilename, &rx_binary);
    trans_model->Read(ki.Stream(), rx_binary);
    am_gmm->Read(ki.Stream(), rx_binary);

    word_syms = NULL;
    if (!(word_syms = fst::SymbolTable::ReadText(word_syms_filename)))
    {
        KALDI_ERR << "Could not read symbol table from file "
                    << word_syms_filename;
    }

    decode_fst = ReadDecodeGraph(fst_rxfilename);

    // We are not properly registering/exposing MFCC and frame extraction options,
    // because there are parts of the online decoding code, where some of these
    // options are hardwired(ToDo: we should fix this at some point)
    MfccOptions mfcc_opts;
    mfcc_opts.use_energy = false;
    int32 frame_length = mfcc_opts.frame_opts.frame_length_ms = 25;
    int32 frame_shift = mfcc_opts.frame_opts.frame_shift_ms = 10;

    int32 window_size = right_context + left_context + 1;
    decoder_opts.batch_size = std::max(decoder_opts.batch_size, window_size);
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

void KdOnline::startDecode()
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
    bool partial_res = false;

    while(1)
    {
        OnlineFasterDecoder::DecodeState dstate = decoder->Decode(&decodable);

        std::vector<int32> word_ids;
        if ( dstate&decoder->kEndUtt )
        {
            decoder->FinishTraceBack(&out_fst);
            fst::GetLinearSymbolSequence(out_fst,
                                         static_cast<vector<int32> *>(0),
                                         &word_ids,
                                         static_cast<LatticeArc::Weight*>(0));
            PrintPartialResult(word_ids, word_syms, partial_res || word_ids.size());
            partial_res = false;
        }
        else
        {
            if (decoder->PartialTraceback(&out_fst))
            {
                fst::GetLinearSymbolSequence(out_fst,
                                           static_cast<vector<int32> *>(0),
                                           &word_ids,
                                           static_cast<LatticeArc::Weight*>(0));
                PrintPartialResult(word_ids, word_syms, false);
                if( (partial_res==0) && (word_ids.size()>0) )
                {
                    partial_res = 1;
                }
            }
        }
    }
}
