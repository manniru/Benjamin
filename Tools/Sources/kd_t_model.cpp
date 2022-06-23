#include "kd_t_model.h"
#include "tree/context-dep.h"
#include <QDebug>

using namespace kaldi;

void KdTransitionModel::ComputeDerived()
{
    state2id_.resize(t_states.size()+2);  // indexed by transition-state, which
    // is one based, but also an entry for one past end of list.

    int cur_transition_id = 1;
    num_pdfs = 0;
    for (int tstate = 1;
         tstate <= static_cast<int>(t_states.size()+1);  // not a typo.
         tstate++) {
        state2id_[tstate] = cur_transition_id;
        if (static_cast<size_t>(tstate) <= t_states.size()) {
            int phone = t_states[tstate-1].phone,
                    hmm_state = t_states[tstate-1].hmm_state,
                    forward_pdf = t_states[tstate-1].forward_pdf,
                    self_loop_pdf = t_states[tstate-1].self_loop_pdf;
            num_pdfs = std::max(num_pdfs, 1 + forward_pdf);
            num_pdfs = std::max(num_pdfs, 1 + self_loop_pdf);
            const HmmTopology::HmmState &state = topo_.TopologyForPhone(phone)[hmm_state];
            int my_num_ids = static_cast<int>(state.transitions.size());
            cur_transition_id += my_num_ids;  // # trans out of this state.
        }
    }

    id2state_.resize(cur_transition_id);   // cur_transition_id is #transition-ids+1.
    id2pdf.resize(cur_transition_id);
    for (int tstate = 1; tstate <= static_cast<int>(t_states.size()); tstate++) {
        for (int tid = state2id_[tstate]; tid < state2id_[tstate+1]; tid++) {
            id2state_[tid] = tstate;
            if (IsSelfLoop(tid))
                id2pdf[tid] = t_states[tstate-1].self_loop_pdf;
            else
                id2pdf[tid] = t_states[tstate-1].forward_pdf;
        }
    }

    // The following statements put copies a large number in the region of memory
    // past the end of the id2pdf_id_ array, while leaving the array as it was
    // before.  The goal of this is to speed up decoding by disabling a check
    // inside TransitionIdToPdf() that the transition-id was within the correct
    // range.
    int num_big_numbers = std::min<int>(2000, cur_transition_id);
    id2pdf.resize(cur_transition_id + num_big_numbers,
                  std::numeric_limits<int>::max());
    id2pdf.resize(cur_transition_id);
}

void KdTransitionModel::InitializeProbs() {
    log_probs_.Resize(NumTransitionIds()+1);  // one-based array, zeroth element empty.
    for (int trans_id = 1; trans_id <= NumTransitionIds(); trans_id++) {
        int trans_state = id2state_[trans_id];
        int trans_index = trans_id - state2id_[trans_state];
        const KdTuple &tuple = t_states[trans_state-1];
        const HmmTopology::TopologyEntry &entry = topo_.TopologyForPhone(tuple.phone);
        KALDI_ASSERT(static_cast<size_t>(tuple.hmm_state) < entry.size());
        float prob = entry[tuple.hmm_state].transitions[trans_index].second;
        if (prob <= 0.0)
            KALDI_ERR << "KdTransitionModel::InitializeProbs, zero "
                         "probability [should remove that entry in the topology]";
        if (prob > 1.0)
            KALDI_WARN << "KdTransitionModel::InitializeProbs, prob greater than one.";
        log_probs_(trans_id) = Log(prob);
    }
}

void KdTransitionModel::Read(std::istream &is, bool binary)
{
    ExpectToken(is, binary, "<TransitionModel>");
    topo_.Read(is, binary);
    std::string token;
    ReadToken(is, binary, &token);
    int size;
    ReadBasicType(is, binary, &size);
    t_states.resize(size);
    for (int i = 0; i < size; i++) {
        ReadBasicType(is, binary, &(t_states[i].phone));
        ReadBasicType(is, binary, &(t_states[i].hmm_state));
        ReadBasicType(is, binary, &(t_states[i].forward_pdf));
        if (token == "<Tuples>")
            ReadBasicType(is, binary, &(t_states[i].self_loop_pdf));
        else if (token == "<Triples>")
            t_states[i].self_loop_pdf = t_states[i].forward_pdf;
    }
    ReadToken(is, binary, &token);
    KALDI_ASSERT(token == "</Triples>" || token == "</Tuples>");
    ComputeDerived();
    ExpectToken(is, binary, "<LogProbs>");
    log_probs_.Read(is, binary);
    ExpectToken(is, binary, "</LogProbs>");
    ExpectToken(is, binary, "</TransitionModel>");
}

int KdTransitionModel::TransitionIdToPhone(int trans_id) const {
    KALDI_ASSERT(trans_id != 0 && static_cast<size_t>(trans_id) < id2state_.size());
    int trans_state = id2state_[trans_id];
    return t_states[trans_state-1].phone;
}

int KdTransitionModel::TransitionIdToHmmState(int trans_id) const
{
    int trans_state = id2state_[trans_id];
    const KdTuple &t = t_states[trans_state-1];
    return t.hmm_state;
}

bool KdTransitionModel::IsSelfLoop(int trans_id) const
{
    int trans_state = id2state_[trans_id];
    int trans_index = trans_id - state2id_[trans_state];
    const KdTuple &tuple = t_states[trans_state-1];
    int phone = tuple.phone, hmm_state = tuple.hmm_state;
    const HmmTopology::TopologyEntry &entry = topo_.TopologyForPhone(phone);
    return (static_cast<size_t>(trans_index) < entry[hmm_state].transitions.size()
            && entry[hmm_state].transitions[trans_index].first == hmm_state);
}

int KdTransitionModel::NumTransitionIds()
{
    return id2state_.size()-1;
}
