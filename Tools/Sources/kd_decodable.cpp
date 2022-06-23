#include "kd_decodable.h"
#include <QDebug>

KdDecodable::KdDecodable(BtCyclic *buf, KdAModel *a_mdl,
                         KdTransitionModel *t_mdl, float scale)
{
    ac_scale_ = scale;
    cur_frame_ = -1;

    ac_model = a_mdl;
    trans_model = t_mdl;

    int num_pdfs = trans_model->num_pdfs;
    cache_.resize(num_pdfs, std::pair<int,float>(-1, 0.0f));

    features = new BtFeInput(buf);
}

KdDecodable::~KdDecodable()
{
    delete features;
}

void KdDecodable::CacheFeature(uint frame)
{
    features->computeFrame(frame);
    feat_buf = features->o_features->get(frame);
    cur_frame_ = frame;
}

float KdDecodable::LogLikelihood(uint frame, int index)
{
    if( frame!=cur_frame_ )
    {
        CacheFeature(frame);
    }

    int pdf_id = trans_model->id2pdf[index];

    if( cache_[pdf_id].first==frame)
    {
        return cache_[pdf_id].second;
    }

    float ans = ac_model->LogLikelihood(pdf_id, feat_buf) * ac_scale_;
    cache_[pdf_id].first = frame;
    cache_[pdf_id].second = ans;

    return ans;
}

uint KdDecodable::NumFramesReady()
{
    return features->NumFramesReady();
}

int KdDecodable::NumIndices()
{
    return trans_model->NumTransitionIds();
}
