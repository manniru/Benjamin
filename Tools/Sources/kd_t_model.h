#ifndef KD_T_MODEL_H
#define KD_T_MODEL_H

#include "base/kaldi-common.h"
#include "util/const-integer-set.h"
#include "fst/fst-decl.h" // forward declarations.
#include "hmm/hmm-topology.h"
#include "itf/options-itf.h"
#include "itf/context-dep-itf.h"
#include "matrix/kaldi-vector.h"
#include "bt_cfb.h"
#include "kd_gmm.h"

class KdTransitionModel
{

public:
    KdTransitionModel(): num_pdfs_(0) { }

    void Read(std::istream &is, bool binary);  // note, no symbol table: topo object always read/written w/o symbols.

    int32 TupleToTransitionState(int32 phone, int32 hmm_state, int32 pdf, int32 self_loop_pdf) const;
    int32 PairToTransitionId(int32 trans_state, int32 trans_index) const;
    int32 TransitionIdToTransitionState(int32 trans_id) const;
    int32 TransitionIdToTransitionIndex(int32 trans_id) const;
    int32 TransitionStateToPhone(int32 trans_state) const;
    int32 TransitionStateToHmmState(int32 trans_state) const;
    int32 TransitionStateToForwardPdfClass(int32 trans_state) const;
    int32 TransitionStateToSelfLoopPdfClass(int32 trans_state) const;
    int32 TransitionStateToForwardPdf(int32 trans_state) const;
    int32 TransitionStateToSelfLoopPdf(int32 trans_state) const;
    int32 SelfLoopOf(int32 trans_state) const;  // returns the self-loop transition-id, or zero if
    // this state doesn't have a self-loop.

    int32 TransitionIdToPhone(int32 trans_id) const;
    int32 TransitionIdToPdfClass(int32 trans_id) const;
    int32 TransitionIdToHmmState(int32 trans_id) const;

    /// @}

    bool IsFinal(int32 trans_id) const;  // returns true if this trans_id goes to the final state
    // (which is bound to be nonemitting).
    bool IsSelfLoop(int32 trans_id) const;  // return true if this trans_id corresponds to a self-loop.

    /// Returns the total number of transition-ids (note, these are one-based).
    inline int32 NumTransitionIds() const { return id2state_.size()-1; }

    /// Returns the number of transition-indices for a particular transition-state.
    /// Note: "Indices" is the plural of "index".   Index is not the same as "id",
    /// here.  A transition-index is a zero-based offset into the transitions
    /// out of a particular transition state.
    int32 NumTransitionIndices(int32 trans_state) const;

    /// Returns the total number of transition-states (note, these are one-based).
    int32 NumTransitionStates() const { return tuples_.size(); }

    // NumPdfs() actually returns the highest-numbered pdf we ever saw, plus one.
    // In normal cases this should equal the number of pdfs in the system, but if you
    // initialized this object with fewer than all the phones, and it happens that
    // an unseen phone has the highest-numbered pdf, this might be different.
    int32 NumPdfs() const { return num_pdfs_; }

    // This loops over the tuples and finds the highest phone index present. If
    // the FST symbol table for the phones is created in the expected way, i.e.:
    // starting from 1 (<eps> is 0) and numbered contiguously till the last phone,
    // this will be the total number of phones.
    int32 NumPhones() const;
    std::vector<int32> id2pdf;

private:
    void ComputeDerived();  // called from constructor and Read function: computes state2id_ and id2state_.
    void ComputeDerivedOfProbs();  // computes quantities derived from log-probs (currently just
    // non_self_loop_log_probs_; called whenever log-probs change.
    void InitializeProbs();  // called from constructor.
    void Check() const;
    bool IsHmm() const;

    struct Tuple {
        int32 phone;
        int32 hmm_state;
        int32 forward_pdf;
        int32 self_loop_pdf;
        Tuple() { }
        Tuple(int32 phone, int32 hmm_state, int32 forward_pdf, int32 self_loop_pdf):
            phone(phone), hmm_state(hmm_state), forward_pdf(forward_pdf), self_loop_pdf(self_loop_pdf) { }
        bool operator < (const Tuple &other) const {
            if (phone < other.phone) return true;
            else if (phone > other.phone) return false;
            else if (hmm_state < other.hmm_state) return true;
            else if (hmm_state > other.hmm_state) return false;
            else if (forward_pdf < other.forward_pdf) return true;
            else if (forward_pdf > other.forward_pdf) return false;
            else return (self_loop_pdf < other.self_loop_pdf);
        }
        bool operator == (const Tuple &other) const {
            return (phone == other.phone && hmm_state == other.hmm_state
                    && forward_pdf == other.forward_pdf && self_loop_pdf == other.self_loop_pdf);
        }
    };

    kaldi::HmmTopology topo_;

    /// Tuples indexed by transition state minus one;
    /// the tuples are in sorted order which allows us to do the reverse mapping from
    /// tuple to transition state
    std::vector<Tuple> tuples_;

    /// Gives the first transition_id of each transition-state; indexed by
    /// the transition-state.  Array indexed 1..num-transition-states+1 (the last one
    /// is needed so we can know the num-transitions of the last transition-state.
    std::vector<int32> state2id_;

    /// For each transition-id, the corresponding transition
    /// state (indexed by transition-id).
    std::vector<int32> id2state_;

    /// For each transition-id, the corresponding log-prob.  Indexed by transition-id.
    kaldi::Vector<float> log_probs_;

    /// This is actually one plus the highest-numbered pdf we ever got back from the
    /// tree (but the tree numbers pdfs contiguously from zero so this is the number
    /// of pdfs).
    int32 num_pdfs_;
};

#endif // KD_T_MODEL_H
