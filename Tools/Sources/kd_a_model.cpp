#include "kd_a_model.h"

using namespace kaldi;

KdAModel::KdAModel()
{
    ;
}

KdAModel::~KdAModel()
{
    DeletePointers(&densities_);
}

void KdAModel::Read(std::istream &in_stream, bool binary)
{
    int32 num_pdfs, dim;

    ExpectToken(in_stream, binary, "<DIMENSION>");
    ReadBasicType(in_stream, binary, &dim);
    ExpectToken(in_stream, binary, "<NUMPDFS>");
    ReadBasicType(in_stream, binary, &num_pdfs);
    KALDI_ASSERT(num_pdfs > 0);
    densities_.reserve(num_pdfs);
    for (int32 i = 0; i < num_pdfs; i++)
    {
        densities_.push_back(new DiagGmm());
        densities_.back()->Read(in_stream, binary);
        KALDI_ASSERT(densities_.back()->Dim() == dim);
    }
}

float KdAModel::LogLikelihood(int32 pdf_index, BtFrameBuf *buf)
{
    Vector<float> feat_buf;
    feat_buf.Resize(BT_DELTA_SIZE);
    for( int i=0 ; i<BT_DELTA_SIZE ; i++ )
    {
        feat_buf(i) = buf->delta[i];
    }
    return densities_[pdf_index]->LogLikelihood(feat_buf);
}
