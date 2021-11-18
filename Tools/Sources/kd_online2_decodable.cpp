#include "kd_online2_decodable.h"

using namespace kaldi;
using namespace fst;

KdOnline2Decodable::KdOnline2Decodable(KdOnline2Model *mdl,
      float scale,Matrix<BaseFloat> transform)
{
    ac_scale_ = scale;
    cur_frame_ = -1;

    ac_model = mdl->GetOnlineAlignmentModel();
    trans_model = mdl->GetTransitionModel();

    int32 num_pdfs = trans_model->NumPdfs();
    cache_.resize(num_pdfs, std::pair<int32,BaseFloat>(-1, 0.0f));

    features = new KdOnline2FeInput;
    feat_dim = features->Dim();
    cur_feats_.Resize(feat_dim);
    features->SetTransform(transform);
}

void KdOnline2Decodable::CacheFrame(int32 frame)
{
    // The call below will fail if "frame" is an invalid index, i.e. <0
    // or >= features_->NumFramesReady(), so there
    // is no need to check again.
    features->GetFrame(frame, &cur_feats_);
    cur_frame_ = frame;
}

BaseFloat KdOnline2Decodable::LogLikelihood(int32 frame, int32 index)
{
    if (frame != cur_frame_)
    {
        CacheFrame(frame);
    }

    int32 pdf_id = trans_model->TransitionIdToPdf(index);
    if (cache_[pdf_id].first == frame)
    {
        return cache_[pdf_id].second;
    }

    BaseFloat ans = ac_model->LogLikelihood(pdf_id, cur_feats_) * ac_scale_;
    cache_[pdf_id].first = frame;
    cache_[pdf_id].second = ans;
    return ans;
}


bool KdOnline2Decodable::IsLastFrame(int32 frame) const
{
    return features->IsLastFrame(frame);
}

int32 KdOnline2Decodable::NumFramesReady() const
{
    return features->NumFramesReady();
}

int32 KdOnline2Decodable::NumIndices() const
{
    return trans_model->NumTransitionIds();
}

