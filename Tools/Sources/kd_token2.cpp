#include "kd_token2.h"
#include <QDebug>

using namespace kaldi;
KdToken2::KdToken2(float total_cost, float e_cost, KdToken2 *next_tok)
{
    tot_cost = total_cost;
    extra_cost = e_cost;
    links = NULL;
    next = next_tok;
    state = KD_INVALID_STATE;
}
