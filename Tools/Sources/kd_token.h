#ifndef KD_TOKEN_H
#define KD_TOKEN_H

#include "kd_lattice.h"

template <typename Token>
struct KdForwardLink
{
  using Label = fst::StdArc::Label;

  Token *next_tok;  // the next token [or NULL if represents final-state]
  Label ilabel;  // ilabel on arc
  Label olabel;  // olabel on arc
  float graph_cost;  // graph cost of traversing arc (contains LM, etc.)
  float acoustic_cost;  // acoustic cost (pre-scaled) of traversing arc
  KdForwardLink *next;  // next in singly-linked list of forward arcs (arcs
                      // in the state-level lattice) from a token.
  inline KdForwardLink(Token *next_tok, Label ilabel, Label olabel,
                     float graph_cost, float acoustic_cost,
                     KdForwardLink *next):
      next_tok(next_tok), ilabel(ilabel), olabel(olabel),
      graph_cost(graph_cost), acoustic_cost(acoustic_cost),
      next(next) { }
};

class KdToken
{
public:
    using ForwardLinkT = KdForwardLink<KdToken>;
    // tot_cost is the total (LM + acoustic) cost from the beginning of the
    // utterance up to this point.
    float tot_cost;
    float extra_cost; // always >= 0

    ForwardLinkT *links;
    KdToken *next; // used for process states(emitting and non-emitting)
    KdToken *prev; // used for MakeLattice
    KdStateId m_state; // used for MakeLattice
    KdStateId state; // used for Decoding
    int ilabel;  // ilabel on arc
    int olabel;  // olabel on arc
    float graph_cost;  // graph cost of traversing arc (contains LM, etc.)
    float acoustic_cost;  // acoustic cost (pre-scaled) of traversing arc

    KdToken(float tot_cost, float extra_cost);
};

/*extra_cost is used in pruning tokens, to save memory.

  extra_cost can be thought of as a beta (backward) cost assuming
  we had set the betas on currently-active tokens to all be the negative
  of the alphas for those tokens.  (So all currently active tokens would
  be on (tied) best paths).

  We can use the extra_cost to accurately prune away tokens that we know will
  never appear in the lattice.  If the extra_cost is greater than the desired
  lattice beam, the token would provably never appear in the lattice, so we can
  prune away the token.

  (Note: we don't update all the extra_costs every time we update a frame; we
  only do it every 'config_.prune_interval' frames).*/
typedef KdForwardLink<KdToken> KdFLink;
#endif // KD_TOKEN_H
