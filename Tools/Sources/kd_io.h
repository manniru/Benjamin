#ifndef KD_IO_H
#define KD_IO_H

#include "base/kaldi-common.h"
#include "matrix/matrix-lib.h"
#include "kd_matrix.h"
#include <QDebug>

std::vector<float> kd_VectorRead(std::istream &is);
KdMatrix kd_MatrixRead(std::istream &is);

// Template that covers integers.
template<class T> inline void kd_ReadBasicType(std::istream &is, T *t)
{
    is.get();
    is.read((char *)(t), sizeof(*t));
    if (is.fail())
    {
        qDebug() << "Read failure in ReadBasicType, file position is "
                 << is.tellg() << ", next char is " << is.peek();
    }
}

void kd_ReadIntegerVector(std::istream &is, std::vector<int> *v);

void kd_openFile(std::string path, std::ifstream *is);
void kd_ReadToken(std::istream &is, std::string *str);
void kd_ExpectToken(std::istream &is, const char *token);

#endif // KD_IO_H
