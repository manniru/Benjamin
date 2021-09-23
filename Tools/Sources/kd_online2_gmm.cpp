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
    d_config.acoustic_scale = 0.05;

    endpoint_config.silence_phones = "1:2:3:4:5:6:7:8:9:10";

    std::string fst_rxfilename = KAL_NATO_DIR"exp/"KAL_MODE"/graph/HCLG.fst";

    OnlineFeaturePipelineConfig feature_config(fcc);
    feature_pipeline_ = new OnlineFeaturePipeline(feature_config);
    // The following object initializes the models we use in decoding.
    models_ = new OnlineGmmDecodingModels(d_config);

    Fst<StdArc> *decode_fst2 = ReadFstKaldiGeneric(fst_rxfilename);

    decoder_ = new LatticeFasterOnlineDecoder(*decode_fst2, d_config.faster_decoder_opts);

    if(!SplitStringToIntegers(d_config.silence_phones, ":", false,
                              &silence_phones_))
    {
        KALDI_ERR << "Bad --silence-phones option '"
              << d_config.silence_phones << "'";
    }

    SortAndUniq(&silence_phones_);
    feature_pipeline_->SetTransform(adaptation_state_.transform);
    decoder_->InitDecoding();
}

// Advance the decoding as far as we can, and possibly estimate fMLLR.
void KdOnline2Gmm::AdvanceDecoding()
{
    const AmDiagGmm &am_gmm = (HaveTransform() ? models_->GetModel() :
                                                 models_->GetOnlineAlignmentModel());

    // The decodable object is lightweight, we lose nothing
    // from constructing it each time we want to decode more of the
    // input.
    DecodableDiagGmmScaledOnline decodable(am_gmm,
                                           models_->GetTransitionModel(),
                                           d_config.acoustic_scale,
                                           feature_pipeline_);

    int32 old_frames = decoder_->NumFramesDecoded();

    // This will decode as many frames as are currently available.
    decoder_->AdvanceDecoding(&decodable);


    // possibly estimate fMLLR.
    int32 new_frames = decoder_->NumFramesDecoded();
    BaseFloat frame_shift = feature_pipeline_->FrameShiftInSeconds();
    // if the original adaptation state (at utterance-start) had no transform,
    // then this means it's the first utt of the speaker... even if not, if we
    // don't have a transform it probably makes sense to treat it as the 1st utt
    // of the speaker, i.e. to do fMLLR adaptation sooner.
    bool is_first_utterance_of_speaker =
            (orig_adaptation_state_.transform.NumRows() == 0);
    bool end_of_utterance = false;
    if (d_config.adaptation_policy_opts.DoAdapt(old_frames * frame_shift,
                                                new_frames * frame_shift,
                                                is_first_utterance_of_speaker))
    {
        this->EstimateFmllr(end_of_utterance);
    }
}

void KdOnline2Gmm::FinalizeDecoding()
{
    decoder_->FinalizeDecoding();
}

// gets Gaussian posteriors for purposes of fMLLR estimation.
// We exclude the silence phones from the Gaussian posteriors.
bool KdOnline2Gmm::GetGaussianPosteriors(bool end_of_utterance,
                                         GaussPost *gpost) {
    // Gets the Gaussian-level posteriors for this utterance, using whatever
    // features and model we are currently decoding with.  We'll use these
    // to estimate basis-fMLLR with.
    if (decoder_->NumFramesDecoded() == 0) {
        KALDI_WARN << "You have decoded no data so cannot estimate fMLLR.";
        return false;
    }

    KALDI_ASSERT(d_config.fmllr_lattice_beam > 0.0);

    // Note: we'll just use whatever acoustic scaling factor we were decoding
    // with.  This is in the lattice that we get from decoder_->GetRawLattice().
    Lattice raw_lat;
    decoder_->GetRawLatticePruned(&raw_lat, end_of_utterance,
                                  d_config.fmllr_lattice_beam);

    // At this point we could rescore the lattice if we wanted, and
    // this might improve the accuracy on long utterances that were
    // the first utterance of that speaker, if we had already
    // estimated the fMLLR by the time we reach this code (e.g. this
    // was the second call).  We don't do this right now.

    PruneLattice(d_config.fmllr_lattice_beam, &raw_lat);

#if 1 // Do determinization.
    Lattice det_lat; // lattice-determinized lattice-- represent this as Lattice
    // not CompactLattice, as LatticeForwardBackward() does not
    // accept CompactLattice.


    fst::Invert(&raw_lat); // want to determinize on words.
    fst::ILabelCompare<kaldi::LatticeArc> ilabel_comp;
    fst::ArcSort(&raw_lat, ilabel_comp); // improves efficiency of determinization

    fst::DeterminizeLatticePruned(raw_lat,
                                  double(d_config.fmllr_lattice_beam),
                                  &det_lat);

    fst::Invert(&det_lat); // invert back.

    if (det_lat.NumStates() == 0) {
        // Do nothing if the lattice is empty.  This should not happen.
        KALDI_WARN << "Got empty lattice.  Not estimating fMLLR.";
        return false;
    }
#else
    Lattice &det_lat = raw_lat; // Don't determinize.
#endif
    TopSortLatticeIfNeeded(&det_lat);

    // Note: the acoustic scale we use here is whatever we decoded with.
    Posterior post;
    BaseFloat tot_fb_like = LatticeForwardBackward(det_lat, &post);

    KALDI_VLOG(3) << "Lattice forward-backward likelihood was "
                << (tot_fb_like / post.size()) << " per frame over " << post.size()
                << " frames.";

    ConstIntegerSet<int32> silence_set(silence_phones_);  // faster lookup
    const TransitionModel &trans_model = models_->GetTransitionModel();
    WeightSilencePost(trans_model, silence_set,
                      d_config.silence_weight, &post);

    const AmDiagGmm &am_gmm = (HaveTransform() ? models_->GetModel() :
                                                 models_->GetOnlineAlignmentModel());


    Posterior pdf_post;
    ConvertPosteriorToPdfs(trans_model, post, &pdf_post);

    Vector<BaseFloat> feat(feature_pipeline_->Dim());

    double tot_like = 0.0, tot_weight = 0.0;
    gpost->resize(pdf_post.size());
    for (size_t i = 0; i < pdf_post.size(); i++) {
        feature_pipeline_->GetFrame(i, &feat);
        for (size_t j = 0; j < pdf_post[i].size(); j++) {
            int32 pdf_id = pdf_post[i][j].first;
            BaseFloat weight = pdf_post[i][j].second;
            const DiagGmm &gmm = am_gmm.GetPdf(pdf_id);
            Vector<BaseFloat> this_post_vec;
            BaseFloat like = gmm.ComponentPosteriors(feat, &this_post_vec);
            this_post_vec.Scale(weight);
            tot_like += like * weight;
            tot_weight += weight;
            (*gpost)[i].push_back(std::make_pair(pdf_id, this_post_vec));
        }
    }
    KALDI_VLOG(3) << "Average likelihood weighted by posterior was "
                << (tot_like / tot_weight) << " over " << tot_weight
                << " frames (after downweighting silence).";
    return true;
}


void KdOnline2Gmm::EstimateFmllr(bool end_of_utterance) {
    if (decoder_->NumFramesDecoded() == 0) {
        KALDI_WARN << "You have decoded no data so cannot estimate fMLLR.";
    }

    if (GetVerboseLevel() >= 2) {
        Matrix<BaseFloat> feats;
        feature_pipeline_->GetAsMatrix(&feats);
        KALDI_VLOG(2) << "Features are " << feats;
    }


    GaussPost gpost;
    GetGaussianPosteriors(end_of_utterance, &gpost);

    FmllrDiagGmmAccs &spk_stats = adaptation_state_.spk_stats;

    if (spk_stats.beta_ !=
            orig_adaptation_state_.spk_stats.beta_) {
        // This could happen if the user called EstimateFmllr() twice on the
        // same utterance... we don't want to count any stats twice so we
        // have to reset the stats to what they were before this utterance
        // (possibly empty).
        spk_stats = orig_adaptation_state_.spk_stats;
    }

    int32 dim = feature_pipeline_->Dim();
    if (spk_stats.Dim() == 0)
        spk_stats.Init(dim);

    Matrix<BaseFloat> empty_transform;
    feature_pipeline_->SetTransform(empty_transform);
    Vector<BaseFloat> feat(dim);

    if (adaptation_state_.transform.NumRows() == 0)
    {
        // If this is the first time we're estimating fMLLR, freeze the CMVN to its
        // current value.  It doesn't matter too much what value this is, since we
        // have already computed the Gaussian-level alignments (it may have a small
        // effect if the basis is very small and doesn't include an offset as part
        // of the transform).
        feature_pipeline_->FreezeCmvn();
    }

    // GetModel() returns the model to be used for estimating
    // transforms.
    const AmDiagGmm &am_gmm = models_->GetModel();

    for (size_t i = 0; i < gpost.size(); i++) {
        feature_pipeline_->GetFrame(i, &feat);
        for (size_t j = 0; j < gpost[i].size(); j++) {
            int32 pdf_id = gpost[i][j].first; // caution: this gpost has pdf-id
            // instead of transition-id, which is
            // unusual.
            const Vector<BaseFloat> &posterior(gpost[i][j].second);
            spk_stats.AccumulateFromPosteriors(am_gmm.GetPdf(pdf_id),
                                               feat, posterior);
        }
    }

    const BasisFmllrEstimate &basis = models_->GetFmllrBasis();
    if (basis.Dim() == 0)
        KALDI_ERR << "In order to estimate fMLLR, you need to supply the "
              << "--fmllr-basis option.";
    Vector<BaseFloat> basis_coeffs;
    BaseFloat impr = basis.ComputeTransform(spk_stats,
                                            &adaptation_state_.transform,
                                            &basis_coeffs, d_config.basis_opts);
    KALDI_VLOG(3) << "Objective function improvement from basis-fMLLR is "
                << (impr / spk_stats.beta_) << " per frame, over "
                << spk_stats.beta_ << " frames, #params estimated is "
                << basis_coeffs.Dim();
    feature_pipeline_->SetTransform(adaptation_state_.transform);
}


bool KdOnline2Gmm::HaveTransform() const
{
    return (feature_pipeline_->HaveFmllrTransform());
}

void KdOnline2Gmm::GetAdaptationState(
        OnlineGmmAdaptationState *adaptation_state)
{
    //    adaptation_state = &adaptation_state_;
    feature_pipeline_->GetCmvnState(&adaptation_state->cmvn_state);
}

bool KdOnline2Gmm::RescoringIsNeeded()
{
    if (orig_adaptation_state_.transform.NumRows() !=
            adaptation_state_.transform.NumRows())
    {
        return true;  // fMLLR was estimated
    }
    if (!orig_adaptation_state_.transform.ApproxEqual(
                adaptation_state_.transform))
    {
        return true;  // fMLLR was re-estimated
    }
    if (adaptation_state_.transform.NumRows() != 0 &&
            &models_->GetModel() != &models_->GetFinalModel())
    {
        return true; // we have an fMLLR transform, and a discriminatively estimated
        // model which differs from the one used to estimate fMLLR.
    }
    return false;

}

KdOnline2Gmm::~KdOnline2Gmm()
{
    delete feature_pipeline_;
}

void KdOnline2Gmm::GetLattice(bool rescore_if_needed,
                              bool end_of_utterance,
                              CompactLattice *clat)
{
    Lattice lat;
    double lat_beam = d_config.faster_decoder_opts.lattice_beam;
    decoder_->GetRawLattice(&lat, end_of_utterance);
    if (rescore_if_needed && RescoringIsNeeded()) {
        DecodableDiagGmmScaledOnline decodable(models_->GetFinalModel(),
                                               models_->GetTransitionModel(),
                                               d_config.acoustic_scale,
                                               feature_pipeline_);

        if (!kaldi::RescoreLattice(&decodable, &lat))
            KALDI_WARN << "Error rescoring lattice";
    }
    PruneLattice(lat_beam, &lat);

    DeterminizeLatticePhonePrunedWrapper(models_->GetTransitionModel(),
                                         &lat, lat_beam, clat,
                                         d_config.faster_decoder_opts.det_opts);

}

#endif
