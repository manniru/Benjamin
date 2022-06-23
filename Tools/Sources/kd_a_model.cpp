#include "kd_a_model.h"
#include <QDebug>

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

    kd_ExpectToken(in_stream, "<DIMENSION>");
    kd_ReadBasicType(in_stream, &dim);
    kd_ExpectToken(in_stream, "<NUMPDFS>");
    kd_ReadBasicType(in_stream, &num_pdfs);
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
