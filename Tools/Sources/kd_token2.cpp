#include "kd_token2.h"
#include <QDebug>

using namespace kaldi;
KdToken2::KdToken2(float total_cost, float e_cost, KdFLink *link, KdToken2 *next_tok)
{
    tot_cost = total_cost;
    extra_cost = e_cost;
    links = link;
    next = next_tok;
}

KdFLink::KdFLink(KdToken2 *n_tok, Label in_label, Label out_label,
                 float g_cost, float a_cost, KdFLink *n_link)
 {
      next_tok = n_tok;
      in_label = ilabel;
      out_label = olabel;
      graph_cost = g_cost;
      acoustic_cost(a_cost);
      next = n_link;
}
