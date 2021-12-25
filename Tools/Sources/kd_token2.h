#ifndef KD_TOKEN2_H
#define KD_TOKEN2_H

#include "lat/kaldi-lattice.h"
#include "decoder/lattice-faster-decoder.h"

#define KD_INVALID_STATE  -1  // Not a valid state ID.
#define KD_INVALID_ARC    -1  // Null Arc
typedef fst::StdFst        KdFST;
typedef fst::StdFst::Arc   KdArc;
typedef KdArc::StateId     KdStateId;
class KdToken2
{
public:
    using ForwardLinkT = kaldi::decoder::ForwardLink<KdToken2>;
    // tot_cost is the total (LM + acoustic) cost from the beginning of the
    // utterance up to this point.
    float tot_cost;
    float extra_cost; // always >= 0

    ForwardLinkT *links;
    KdToken2 *link_tok;
    int is_linked;
    KdToken2 *next;
    KdStateId state = KD_INVALID_STATE;
    int ilabel;  // ilabel on arc
    int olabel;  // olabel on arc
    float graph_cost;  // graph cost of traversing arc (contains LM, etc.)
    float acoustic_cost;  // acoustic cost (pre-scaled) of traversing arc

    KdToken2(float tot_cost, float extra_cost, KdToken2 *next);
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
typedef kaldi::decoder::ForwardLink<KdToken2> KdFLink;
#endif // KD_TOKEN2_H
