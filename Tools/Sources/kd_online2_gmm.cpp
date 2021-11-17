#include "kd_online2_gmm.h"
#include "lat/lattice-functions.h"
#include "lat/determinize-lattice-pruned.h"

#ifdef BT_ONLINE2
using namespace kaldi;
using namespace fst;

KdOnline2Gmm::KdOnline2Gmm(QObject *parent) : QObject(parent)
{
    OnlineEndpointConfig endpoint_config;
    OnlineFeaturePipelineCommandLineConfig fcc; //feature_cmdline_config

    fcc.mfcc_config = KAL_NATO_DIR"exp/tri1_online/conf/mfcc.conf";
    fcc.cmvn_config = KAL_NATO_DIR"exp/tri1_online/conf/online_cmvn.conf";
    fcc.add_deltas = true;
    fcc.global_cmvn_stats_rxfilename = KAL_NATO_DIR"exp/tri1_online/global_cmvn.stats";

    d_config.fmllr_basis_rxfilename = KAL_NATO_DIR"exp/tri1_online/fmllr.basis";
    d_config.online_alimdl_rxfilename = KAL_NATO_DIR"exp/tri1_online/final.oalimdl";
    d_config.model_rxfilename = KAL_NATO_DIR"exp/"KAL_MODE"/final.mdl";
    d_config.silence_phones = "1:2:3:4:5:6:7:8:9:10";
    d_config.faster_decoder_opts.max_active = 7000;
    d_config.faster_decoder_opts.beam = 12.0;
    d_config.faster_decoder_opts.lattice_beam = 6.0;
    d_config.acoustic_scale = 0.08;

    endpoint_config.silence_phones = "1:2:3:4:5:6:7:8:9:10";

    std::string fst_rxfilename = KAL_NATO_DIR"exp/"KAL_MODE"/graph/HCLG.fst";

    feature_config = new OnlineFeaturePipelineConfig(fcc);
    // The following object initializes the models we use in decoding.

    TransitionModel *trans_model = new TransitionModel;
    AmDiagGmm *am_gmm = new AmDiagGmm;

    bool rx_binary;
    Input ki(d_config.model_rxfilename, &rx_binary);
    trans_model->Read(ki.Stream(), rx_binary);
    am_gmm->Read(ki.Stream(), rx_binary);
    models_ = new KdOnline2Model(trans_model, am_gmm, &d_config);

    decode_fst = ReadFstKaldiGeneric(fst_rxfilename);

    feature_pipeline = NULL;
    o_decoder = NULL;

    if(!SplitStringToIntegers(d_config.silence_phones, ":", false,
                              &silence_phones_))
    {
        KALDI_ERR << "Bad --silence-phones option '"
              << d_config.silence_phones << "'";
    }

    SortAndUniq(&silence_phones_);
}

void KdOnline2Gmm::init()
{
    if( o_decoder ) //decoder not null
    {
        delete feature_pipeline;
        delete o_decoder;
    }

    o_decoder = new KdLatticeDecoder(*decode_fst, d_config.faster_decoder_opts);
    feature_pipeline = new OnlineFeaturePipeline(*feature_config);

    o_decoder->InitDecoding();
    feature_pipeline->SetTransform(adaptation_state_.transform);

    // Decodable is lightweight, lose nothing constructing it each time
    decodable = new KdOnline2Decodable(models_, d_config.acoustic_scale, feature_pipeline);

}

// Advance the decoding as far as we can, and possibly estimate fMLLR.
void KdOnline2Gmm::AdvanceDecoding()
{
    o_decoder->AdvanceDecoding(decodable);
}

void KdOnline2Gmm::FinalizeDecoding()
{
    o_decoder->FinalizeDecoding();
}

KdOnline2Gmm::~KdOnline2Gmm()
{
    delete feature_pipeline;
}

//end_of_utterance
bool KdOnline2Gmm::GetLattice(bool end_of_utterance,
                              CompactLattice *clat)
{
    Lattice lat;
    double lat_beam = d_config.faster_decoder_opts.lattice_beam;
    clock_t start = clock();
    bool ret = o_decoder->GetRawLattice(&lat, end_of_utterance);
    printTime(start);

    if( ret==false )
    {
        return false;
    }

    PruneLattice(lat_beam, &lat);

    DeterminizeLatticePhonePrunedWrapper(*(models_->GetTransitionModel()),
                                         &lat, lat_beam, clat,
                                         d_config.faster_decoder_opts.det_opts);

    return true;
}

#endif
