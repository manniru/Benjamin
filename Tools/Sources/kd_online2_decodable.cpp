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
    max_pdf = 0;
}

KdOnline2Decodable::~KdOnline2Decodable()
{
    delete features;
}

void KdOnline2Decodable::CacheFeature(int frame)
{
    features->GetFrame(frame, &cur_feats_);
    cur_frame_ = frame;

#ifdef BT_TEST_MODE
    QVector<KdPDF *> buf;
    p_vec.push_back(buf);
    if( p_vec.size()<frame )
    {
        qDebug() << "FUCKKKKKKKKKKKKKKKK";
    }
#endif
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

#ifdef BT_TEST_MODE
    if( max_pdf<pdf_id )
    {
        max_pdf = pdf_id;
        qDebug() << max_pdf;
    }
    int phone_id = trans_model->TransitionIdToPhone(index);
    addPDF(frame, pdf_id, phone_id, ans);
#endif
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
    if( p_vec.size()<=frame )
    {
        qDebug() << "FUCKKKKKKKKKKKKKKKK2";
        return;
    }
//    qDebug() << "$$$ frame" << frame
//             << "size" << p_vec.size()
//             << "id"   << id;

    for( int i=0 ; i<p_vec[frame].size() ; i++ )
    {
//        qDebug() << "FLAG1" << i << p_vec[frame].size()
//                 << "p_vec" << p_vec[frame][i]->pdf_id;
        if( p_vec[frame][i]->pdf_id==id )
        {
            p_vec[frame][i]->val = val;
            p_vec[frame][i]->phone_id = phone_id;
            return;
        }
    }

    KdPDF *buf = new KdPDF;
    buf->pdf_id = id;
    buf->phone_id = phone_id;
    buf->val = val;
    p_vec[frame].push_back(buf);
//    qDebug() << "+ add id" << buf->pdf_id
//             << "p_vec size"  << p_vec[frame].size();

}
