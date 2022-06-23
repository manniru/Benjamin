#ifndef KD_T_MODEL_H
#define KD_T_MODEL_H

#include "bt_cfb.h"
#include "kd_gmm.h"
#include "kd_io.h"
#include "kd_hmm.h"

struct KdTuple
{
    int phone;
    int hmm_state;
    int forward_pdf;
    int self_loop_pdf;
    KdTuple() { }
    KdTuple(int phone, int hmm_state, int forward_pdf, int self_loop_pdf):
        phone(phone), hmm_state(hmm_state), forward_pdf(forward_pdf), self_loop_pdf(self_loop_pdf) { }
    bool operator < (const KdTuple &other) const {
        if (phone < other.phone) return true;
        else if (phone > other.phone) return false;
        else if (hmm_state < other.hmm_state) return true;
        else if (hmm_state > other.hmm_state) return false;
        else if (forward_pdf < other.forward_pdf) return true;
        else if (forward_pdf > other.forward_pdf) return false;
        else return (self_loop_pdf < other.self_loop_pdf);
    }
    bool operator == (const KdTuple &other) const {
        return (phone == other.phone && hmm_state == other.hmm_state
                && forward_pdf == other.forward_pdf && self_loop_pdf == other.self_loop_pdf);
    }
};

class KdTransitionModel
{

public:
    KdTransitionModel();

    void Read(std::istream &is);

    int TransitionIdToPhone(int trans_id);
    int TransitionIdToHmmState(int trans_id);
    bool IsSelfLoop(int trans_id);  // return true if this trans_id corresponds to a self-loop.

    /// Returns the total number of transition-ids (note, these are one-based).
    int NumTransitionIds();
    std::vector<int> id2pdf;
    int num_pdfs;

private:
    void ComputeDerived();  // called from constructor and Read function: computes state2id_ and id2state_.

    KdHmmTopology topo_;

    /// Tuples indexed by transition state minus one;
    /// the tuples are in sorted order which allows us to do the reverse mapping from
    /// tuple to transition state
    std::vector<KdTuple> t_states;

    /// Gives the first transition_id of each transition-state; indexed by
    /// the transition-state.  Array indexed 1..num-transition-states+1 (the last one
    /// is needed so we can know the num-transitions of the last transition-state.
    std::vector<int> state2id_;

    /// For each transition-id, the corresponding transition
    /// state (indexed by transition-id).
    std::vector<int> id2state;

    /// For each transition-id, the corresponding log-prob.  Indexed by transition-id.
    std::vector<float> log_probs_;
};

#endif // KD_T_MODEL_H
