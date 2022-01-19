#include "kd_window.h"

using namespace kaldi;

// Round up to nearest power of two
int kd_RoundP2(int n)
{
    KALDI_ASSERT(n > 0);
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n+1;
}
