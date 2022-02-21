#include "kd_online2_decodable.h"
#include <QDebug>

using namespace kaldi;
using namespace fst;

KdOnline2Decodable::KdOnline2Decodable(BtRecorder *au_src, KdOnline2Model *mdl, float scale)
{
    ac_scale_ = scale;
    cur_frame_ = -1;

    ac_model = mdl->GetOnlineAlignmentModel();
    trans_model = mdl->GetTransitionModel();

    int num_pdfs = trans_model->NumPdfs();
    cache_.resize(num_pdfs, std::pair<int,float>(-1, 0.0f));

    features = new KdOnline2FeInput(au_src);
    feat_dim = features->Dim();
    cur_feats_.Resize(feat_dim);
}

KdOnline2Decodable::~KdOnline2Decodable()
{
    delete features;
}

void KdOnline2Decodable::CacheFeature(int frame)
{
    features->GetFrame(frame, &cur_feats_);
    cur_frame_ = frame;

    int index = frame%MAX_FRAME_CNT;
    p_vec[index].val = -100; //dB
}

float KdOnline2Decodable::LogLikelihood(int frame, int index)
{
    if( frame!=cur_frame_ )
    {
        CacheFeature(frame);
    }

    int pdf_id = trans_model->TransitionIdToPdf(index);

    if (cache_[pdf_id].first == frame)
    {
        return cache_[pdf_id].second;
    }

    float ans = ac_model->LogLikelihood(pdf_id, cur_feats_) * ac_scale_;
    cache_[pdf_id].first = frame;
    cache_[pdf_id].second = ans;

    int phone_id = trans_model->TransitionIdToPhone(index);
    addPDF(frame, pdf_id, phone_id, ans);

//    ac_model->

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

void KdOnline2Decodable::addPDF(int frame, int id, int phone_id, float val)
{
    int index = frame%MAX_FRAME_CNT;

    if( p_vec[index].val<val )
    {
        p_vec[index].val = val;
        p_vec[index].phone_id = phone_id;
        p_vec[index].pdf_id = id;
        return;
    }
}

int KdOnline2Decodable::getPhone(int frame)
{
    int index = frame%MAX_FRAME_CNT;

    return p_vec[index].phone_id;
}
