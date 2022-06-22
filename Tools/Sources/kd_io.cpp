#include "kd_io.h"
#include <QDebug>

using namespace kaldi;

// assume float
std::vector<float> kd_VectorRead(std::istream &is)
{
    std::ostringstream specific_error;
    std::vector<float> buf;

    std::string token;
    ReadToken(is, true, &token);
    if( token!="FV" )
    {
        qDebug() << "Expected token FV, got " << token.c_str();
    }
    int size;
    ReadBasicType(is, true, &size);  // throws on error.
    if( size!=buf.size() )
    {
        buf.resize(size);
    }

    for( int i=0 ; i<size ; i++ )
    {
        is.read((char*)(&(buf[i])), sizeof(float));
    }
    if( is.fail() )
    {
        specific_error << "Error reading vector data (true mode); truncated "
                          "stream? (size = " << size << ")";
    }
    return buf;
}

// Not support compressed or double matrix
KdMat kd_MatrixRead(std::istream &is)
{
    KdMat buf;
    // now assume add == false.
    std::string token;
    ReadToken(is, true, &token);
    if( token!="FM" )
    {
        qDebug() << "Expected token FM, got " << token.c_str();
    }
    ReadBasicType(is, true, &buf.rows);  // throws on error.
    ReadBasicType(is, true, &buf.cols);  // throws on error.
    buf.d = (float **)malloc(sizeof(float *)*buf.rows);

    for ( int i=0 ; i<buf.rows ; i++ )
    {
        buf.d[i] = (float *)malloc(sizeof(float)*buf.cols);
        is.read((char*)(buf.d[i]), sizeof(float)*buf.cols);
    }
    if (is.fail())
    {
        qDebug() << "is.fail()";
    }
    return buf;
}

