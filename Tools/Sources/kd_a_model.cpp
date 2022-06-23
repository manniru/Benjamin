#include "kd_a_model.h"
#include <QDebug>

using namespace kaldi;

KdAModel::KdAModel()
{
    ;
}

KdAModel::~KdAModel()
{
    int len = densities.size();
    for( int i=0 ; i<len; i++ )
    {
        if( densities[i]!= NULL)
        {
            delete densities[i];
            densities[i] = NULL;
        }
    }
}

void KdAModel::Read(std::istream &in_stream)
{
    int num_pdfs, dim;

    ExpectToken(in_stream, true, "<DIMENSION>");
    ReadBasicType(in_stream, true, &dim);
    ExpectToken(in_stream, true, "<NUMPDFS>");
    ReadBasicType(in_stream, true, &num_pdfs);
    densities.resize(num_pdfs);
    for( int i=0 ; i<num_pdfs; i++ )
    {
        densities[i] = new KdGmm();
        densities[i]->Read(in_stream);
    }
}

float KdAModel::LogLikelihood(int pdf_index, BtFrameBuf *buf)
{
    return densities[pdf_index]->LogLikelihood(buf->delta, BT_DELTA_SIZE);
}
