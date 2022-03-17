#include "kd_token.h"
#include <QDebug>

using namespace kaldi;
KdToken::KdToken(float total_cost)
{
    cost = total_cost;
    prune_cost = 0;
    links = NULL;
    next = NULL;
    prev = NULL;

    state = KD_INVALID_STATE;
    m_state = KD_INVALID_STATE;
}

//template<typename Token>
//KdForwardLink<Token>::KdForwardLink(Token *n_tok, int in_label, int out_label,
//            float g_cost, float a_cost, KdForwardLink<Token> *next_link)
//{
//    ilabel = in_label;
//    olabel = out_label;
//    acoustic_cost = a_cost;
//    graph_cost = g_cost;
//    next = next_link;
//    next_tok = n_tok;
//}
