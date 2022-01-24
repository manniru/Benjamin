#include "kd_online2_decodable.h"

using namespace kaldi;
using namespace fst;

KdOnline2Decodable::KdOnline2Decodable(BtRecorder *au_src, KdOnline2Model *mdl, float scale)
{
    ac_scale_ = scale;
    cur_frame_ = -1;

    ac_model = mdl->GetOnlineAlignmentModel();
    trans_model = mdl->GetTransitionModel();

    int num_pdfs = trans_model->NumPdfs();
    cache_.resize(num_pdfs, std::pair<int,BaseFloat>(-1, 0.0f));

    features = new KdOnline2FeInput(au_src);
    feat_dim = features->Dim();
    cur_feats_.Resize(feat_dim);
}

KdOnline2Decodable::~KdOnline2Decodable()
{
    delete features;
}

void KdOnline2Decodable::CacheFrame(int frame)
{
    // The call below will fail if "frame" is an invalid index, i.e. <0
    // or >= features_->NumFramesReady(), so there
    // is no need to check again.
    features->GetFrame(frame, &cur_feats_);
    cur_frame_ = frame;
}

float KdOnline2Decodable::LogLikelihood(int frame, int index)
{
    if (frame != cur_frame_)
    {
        CacheFrame(frame);
    }

    int pdf_id = trans_model->TransitionIdToPdf(index);
    if (cache_[pdf_id].first == frame)
    {
        return cache_[pdf_id].second;
    }

    BaseFloat ans = ac_model->LogLikelihood(pdf_id, cur_feats_) * ac_scale_;
    cache_[pdf_id].first = frame;
    cache_[pdf_id].second = ans;
    return ans;
}

int KdOnline2Decodable::NumFramesReady()
{
    return features->NumFramesReady();
}

int KdOnline2Decodable::NumIndices()
{
    return trans_model->NumTransitionIds();
}

