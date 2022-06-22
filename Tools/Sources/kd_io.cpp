#include "kd_io.h"
#include <QDebug>

using namespace kaldi;

// assume float
std::vector<float> kd_VectorRead(std::istream &is)
{
    std::ostringstream specific_error;
    std::vector<float> buf;

    char *my_token =  "FV";
    std::string token;
    ReadToken(is, true, &token);
    if( token!=my_token )
    {
        if( token.size()>20 )
        {
            token = token.substr(0, 17) + "...";
        }
        specific_error << ": Expected token " << my_token << ", got " << token;
    }
    int size;
    ReadBasicType(is, true, &size);  // throws on error.
    if( size!=buf.size() )
    {
        buf.resize(size);
    }
    if( size>0 )
    {
        for( int i=0 ; i<size ; i++ )
        {
            is.read((char*)(&(buf[i])), sizeof(float));
        }
    }
    if( is.fail() )
    {
        specific_error << "Error reading vector data (true mode); truncated "
                          "stream? (size = " << size << ")";
    }
    return buf;
}

// Not support compressed or double matrix
Matrix<float> kd_MatrixRead(std::istream &is)
{
    Matrix<float> buf;
    // now assume add == false.
    std::string token;
    ReadToken(is, true, &token);
    if( token!="FM" )
    {
        qDebug() << ": Expected token FM, got " << token.c_str();
    }
    int32 rows, cols;
    ReadBasicType(is, true, &rows);  // throws on error.
    ReadBasicType(is, true, &cols);  // throws on error.
    buf.Resize(rows, cols);
    for (MatrixIndexT i = 0; i < (MatrixIndexT)rows; i++)
    {
        is.read(reinterpret_cast<char*>(buf.RowData(i)), sizeof(float)*cols);
    }
    if (is.fail())
    {
        qDebug() << "is.fail()";
    }
    return buf;
}

