#ifndef KD_TOKEN_H
#define KD_TOKEN_H

#include "kd_lattice.h"

template <typename Token>
class KdForwardLink
{
public:
    Token *next_tok;  //NULL if final-state
    int   ilabel;
    int   olabel;
    float graph_cost;
    float acoustic_cost;
    KdForwardLink<Token> *next;
    inline KdForwardLink(Token *n_tok, int in_label, int out_label,
           float g_cost, float a_cost, KdForwardLink<Token> *next_link)
    {
        ilabel = in_label;
        olabel = out_label;
        acoustic_cost = a_cost;
        graph_cost = g_cost;
        next = next_link;
        next_tok = n_tok;
    }

};

class KdToken
{
public:
    KdToken(float tot_cost);

    using ForwardLinkT = KdForwardLink<KdToken>;

    // graph + acoustic cost from the beginning of the
    // utterance up to this point.
    float cost;

    // prune_cost is used in pruning tokens. If the prune_cost is greater than the
    // lattice beam, the token would provably never appear in the final latticeez4
    float prune_cost;

    ForwardLinkT *links;
    KdToken *next; // used for process states(emitting and non-emitting)
    KdToken *prev; // used for MakeLattice
    KdStateId m_state; // used for MakeLattice
    KdStateId state; // used for Decoding
    int       tok_id; // used for graph debug
};

typedef KdForwardLink<KdToken> KdFLink;
#endif // KD_TOKEN_H
