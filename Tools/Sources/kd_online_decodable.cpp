#include "kd_online_decodable.h"

using namespace kaldi;
using namespace fst;

KdOnlineDecodable::KdOnlineDecodable(KdOnline2Model *mdl,
                    float scale, OnlineFeatureMatrix *input_feats)
{
    ac_scale_ = scale;
    feat_dim = input_feats->Dim();
    cur_feats_.Resize(feat_dim);
    cur_frame_ = -1;

    ac_model = mdl->GetOnlineAlignmentModel();
    trans_model = mdl->GetTransitionModel();

    features = input_feats;

    int32 num_pdfs = trans_model->NumPdfs();
    cache_.resize(num_pdfs, std::pair<int32,BaseFloat>(-1, 0.0f));
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
