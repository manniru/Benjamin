#ifndef KD_TOKEN_H
#define KD_TOKEN_H

#include "kd_lattice.h"

typedef struct BtTokenArc
{
    int   ilabel;
    int   olabel;
    float graph_cost;
    float acoustic_cost;
}BtTokenArc;

class KdToken
{
public:
    KdToken(float tot_cost);

    // graph + acoustic cost from the beginning of the
    // utterance up to this point.
    float cost;

    // used in pruning tokens. If the prune_cost is greater than the lattice
    // beam, the token would probably not appear in the final lattice
    float prune_cost;

    QVector<BtTokenArc> arc;
    QVector<KdToken*>   arc_ns; // Arc Next state, NULL if final-state

    KdToken *next; // used for process states(emitting and non-emitting)
    KdToken *prev; // used for MakeLattice
    KdStateId m_state; // state in final lattice (used in MakeLattice)
    KdStateId g_state;   // state in LM graph (used in Decoding)
    int       tok_id; // used for graph debug
};
#endif // KD_TOKEN_H
