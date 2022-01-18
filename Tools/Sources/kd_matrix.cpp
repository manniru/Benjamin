#include "kd_matrix.h"

using namespace kaldi;

void kd_readMatrix(std::string &filename, Matrix<float> *c)
{
    bool binary_in;
    Input ki(filename, &binary_in);
    c->Read(ki.Stream(), binary_in);
}
