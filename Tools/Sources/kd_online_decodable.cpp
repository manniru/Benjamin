#include "kd_online_decodable.h"

using namespace kaldi;
using namespace fst;

KdOnlineDecodable::KdOnlineDecodable(BtRecorder *au_src, KdOnline2Model *mdl,
                    float scale)
{
    MfccOptions mfcc_opts;
    mfcc_opts.use_energy = false;
    mfcc_opts.frame_opts.frame_length_ms = KD_FRAME_LENGTH;
    mfcc_opts.frame_opts.frame_shift_ms = KD_FRAME_SHIFT;
    mfcc = new Mfcc(mfcc_opts);
    fe_input = new KdOnlineFeInput(au_src, mfcc,
                     KD_WINDOW_SIZE, KD_FEAT_SIZE);
    cmn_input = new OnlineCmnInput(fe_input, cmn_window, min_cmn_window);

    DeltaFeaturesOptions opts;
    opts.order = kDeltaOrder;
    feat_transform = new OnlineDeltaInput(opts, cmn_input);

    OnlineFeatureMatrixOptions feature_reading_opts;
    features = new OnlineFeatureMatrix(feature_reading_opts,
                                             feat_transform);

    ac_scale_ = scale;
    feat_dim = features->Dim();
    cur_feats_.Resize(feat_dim);
    cur_frame_ = -1;

    ac_model = mdl->GetOnlineAlignmentModel();
    trans_model = mdl->GetTransitionModel();

    int32 num_pdfs = trans_model->NumPdfs();
    cache_.resize(num_pdfs, std::pair<int32,BaseFloat>(-1, 0.0f));
}

KdOnlineDecodable::~KdOnlineDecodable()
{
    delete feat_transform;
    delete cmn_input;
}

void KdOnlineDecodable::CacheFrame(int32 frame)
{
    KALDI_ASSERT(frame >= 0);
    cur_feats_.Resize(feat_dim);
    if (!features->IsValidFrame(frame))
        KALDI_ERR << "Request for invalid frame (you need to check IsLastFrame, or, "
                  << "for frame zero, check that the input is valid.";
    cur_feats_.CopyFromVec(features->GetFrame(frame));
    cur_frame_ = frame;
}

BaseFloat KdOnlineDecodable::LogLikelihood(int32 frame, int32 index)
{
    if (frame != cur_frame_)
        CacheFrame(frame);
    int32 pdf_id = trans_model->TransitionIdToPdf(index);
    if (cache_[pdf_id].first == frame)
        return cache_[pdf_id].second;
    BaseFloat ans = ac_model->LogLikelihood(pdf_id, cur_feats_) * ac_scale_;
    cache_[pdf_id].first = frame;
    cache_[pdf_id].second = ans;
    return ans;
}


bool KdOnlineDecodable::IsLastFrame(int32 frame) const
{
    return !features->IsValidFrame(frame+1);
}
