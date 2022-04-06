#include "kd_decodable.h"
#include <QDebug>

using namespace kaldi;
using namespace fst;

KdDecodable::KdDecodable(BtCyclic *buf, KdModel *mdl, float scale)
{
    ac_scale_ = scale;
    cur_frame_ = -1;

    ac_model = mdl->oa_model;
    trans_model = mdl->t_model;

    int num_pdfs = trans_model->NumPdfs();
    cache_.resize(num_pdfs, std::pair<int,float>(-1, 0.0f));

    features = new BtFeInput(buf);
    feat_dim = features->Dim();
    feat_buf.Resize(feat_dim);
}

KdDecodable::~KdDecodable()
{
    delete features;
}

void KdDecodable::CacheFeature(int frame)
{
    features->GetFrame(frame, &feat_buf);
    cur_frame_ = frame;


    int len = feat_buf.Dim();
    QString buf = QString::number(frame);
    buf += ": ";;
    for( int i=0 ; i<len ; i++ )
    {
        buf += QString::number(feat_buf(i));
        buf += " ";
    }
//    qDebug() << buf;
}

float KdDecodable::LogLikelihood(int frame, int index)
{
    if( frame!=cur_frame_ )
    {
        CacheFeature(frame);
    }

    int pdf_id = trans_model->TransitionIdToPdf(index);

    if( cache_[pdf_id].first == frame)
    {
        return cache_[pdf_id].second;
    }

    float ans = ac_model->LogLikelihood(pdf_id, feat_buf) * ac_scale_;
    cache_[pdf_id].first = frame;
    cache_[pdf_id].second = ans;

    return ans;
}

int KdDecodable::NumFramesReady()
{
    return features->NumFramesReady();
}

int KdDecodable::NumIndices()
{
    return trans_model->NumTransitionIds();
}
