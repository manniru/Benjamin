#include "kd_token2.h"
#include <QDebug>

using namespace kaldi;
KdToken2::KdToken2(float total_cost, float e_cost, ForwardLinkT *link, KdToken2 *next_tok)
{
    tot_cost = total_cost;
    extra_cost = e_cost;
    links = link;
    next = next_tok;
}
