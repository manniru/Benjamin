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

    // tot_cost is the total (LM + acoustic) cost from the beginning of the
    // utterance up to this point.
    float tot_cost;
    float extra_cost; // always >= 0

    KdFLink *links;
    KdToken2 *next;
    KdStateId state = KD_INVALID_STATE;

    KdToken2(float tot_cost, float extra_cost, KdFLink *links, KdToken2 *next);
};

class KdFLink
{
public:
    using Label = fst::StdArc::Label;

    KdToken2 *next_tok;  // the next token [or NULL if represents final-state]
    int ilabel;  // ilabel on arc
    int olabel;  // olabel on arc
    float graph_cost;  // graph cost of traversing arc (contains LM, etc.)
    float acoustic_cost;  // acoustic cost (pre-scaled) of traversing arc
    KdFLink *next;  // next in singly-linked list of forward arcs (arcs
    // in the state-level lattice) from a token.
    KdFLink(KdToken2 *next_tok, int ilabel, int olabel,
            float graph_cost, float acoustic_cost,
            KdFLink *next);
};

#endif // KD_TOKEN2_H
