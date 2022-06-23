#include "kd_io.h"
#include <QDebug>
#include <sstream>

// assume float
std::vector<float> kd_VectorRead(std::istream &is)
{
    std::ostringstream specific_error;
    std::vector<float> buf;

    std::string token;
    kd_ReadToken(is, &token);
    if( token!="FV" )
    {
        qDebug() << "Expected token FV, got " << token.c_str();
    }
    int size;
    kd_ReadBasicType(is, &size);  // throws on error.
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
KdMatrix kd_MatrixRead(std::istream &is)
{
    KdMatrix buf;
    // now assume add == false.
    std::string token;
    kd_ReadToken(is, &token);
    if( token!="FM" )
    {
        qDebug() << "Expected token FM, got " << token.c_str();
    }
    kd_ReadBasicType(is, &buf.rows);  // throws on error.
    kd_ReadBasicType(is, &buf.cols);  // throws on error.
    buf.d = (float **)malloc(sizeof(float *)*buf.rows);

    for(  int i=0 ; i<buf.rows ; i++ )
    {
        buf.d[i] = (float *)malloc(sizeof(float)*buf.cols);
        is.read((char*)(buf.d[i]), sizeof(float)*buf.cols);
    }
    if( is.fail() )
    {
        qDebug() << "is.fail()";
    }
    return buf;
}

void kd_ReadIntegerVector(std::istream &is, std::vector<int> *v)
{
    is.peek();
    is.get();
    int vecsz;
    is.read((char *)(&vecsz), sizeof(vecsz));
    if (is.fail() || vecsz < 0)
    {
        qDebug() << "ReadIntegerVector: read failure at file position "
                 << is.tellg();
        return;
    }
    v->resize(vecsz);
    if( vecsz>0 )
    {
        is.read((char *)(&((*v)[0])), sizeof(int)*vecsz);
    }
    if( !is.fail() )
    {
        return;
    }
}

void kd_openFile(std::string path, std::ifstream *is)
{
    is->open(path.c_str(),
             std::ios_base::in | std::ios_base::binary);
    is->get();
    is->get();
}

void kd_ExpectToken(std::istream &is, const char *token)
{
    int pos_at_start = is.tellg();
    std::string str;
    is >> str;
    is.get();  // consume the space.
    if (is.fail())
    {
        qDebug() << "Failed to read token [started at file position "
                 << pos_at_start << "], expected " << token;
        exit(1);
    }
    // The second half of the '&&' expression below is so that if we're expecting
    // "<Foo>", we will accept "Foo>" instead.  This is so that the model-reading
    // code will tolerate errors in PeekToken where is.unget() failed; search for
    // is.clear() in PeekToken() for an explanation.
    if (strcmp(str.c_str(), token) != 0 &&
            !(token[0] == '<' && strcmp(str.c_str(), token + 1) == 0))
    {
        qDebug() << "Expected token \"" << token << "\", got instead \""
                 << str.c_str() <<"\".";
        exit(1);
    }
}

void kd_ReadToken(std::istream &is, std::string *str)
{
    is >> *str;
    if (is.fail())
    {
        qDebug() << "ReadToken, failed to read token at file position "
                  << is.tellg();
        exit(1);
    }
    if( !isspace(is.peek()) )
    {
        qDebug() << "ReadToken, expected space after token, saw instead "
                  << (char)(is.peek())
                  << ", at file position " << is.tellg();
        exit(1);
    }
    is.get();  // consume the space.
}
