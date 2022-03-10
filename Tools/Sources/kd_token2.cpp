#include "kd_token2.h"
#include <QDebug>

using namespace kaldi;
KdToken2::KdToken2(float total_cost, float e_cost)
{
    tot_cost = total_cost;
    extra_cost = e_cost;
    links = NULL;
    next = NULL;
    prev = NULL;

    state = KD_INVALID_STATE;
    m_state = KD_INVALID_STATE;
}
