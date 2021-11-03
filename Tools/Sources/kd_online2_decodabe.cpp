#include "kd_online2_decodabe.h"

#ifdef BT_ONLINE2
using namespace kaldi;
using namespace fst;

KdOnline2Decodable::KdOnline2Decodable(
    const AmDiagGmm &am, const TransitionModel &trans_model,
    const BaseFloat scale, OnlineFeatureInterface *input_feats):
      features_(input_feats), ac_model_(am),
      ac_scale_(scale), trans_model_(trans_model),
      feat_dim_(input_feats->Dim()), cur_feats_(feat_dim_),
      cur_frame_(-1) {
  int32 num_pdfs = trans_model_.NumPdfs();
  cache_.resize(num_pdfs, std::pair<int32,BaseFloat>(-1, 0.0f));
}

void KdOnline2Decodable::CacheFrame(int32 frame) {
  // The call below will fail if "frame" is an invalid index, i.e. <0
  // or >= features_->NumFramesReady(), so there
  // is no need to check again.
  features_->GetFrame(frame, &cur_feats_);
  cur_frame_ = frame;
}

BaseFloat KdOnline2Decodable::LogLikelihood(int32 frame, int32 index) {
  if (frame != cur_frame_)
    CacheFrame(frame);
  int32 pdf_id = trans_model_.TransitionIdToPdf(index);
  if (cache_[pdf_id].first == frame)
    return cache_[pdf_id].second;
  BaseFloat ans = ac_model_.LogLikelihood(pdf_id, cur_feats_) * ac_scale_;
  cache_[pdf_id].first = frame;
  cache_[pdf_id].second = ans;
  return ans;
}


bool KdOnline2Decodable::IsLastFrame(int32 frame) const {
  return features_->IsLastFrame(frame);
}

int32 KdOnline2Decodable::NumFramesReady() const {
  return features_->NumFramesReady();
}

#endif
