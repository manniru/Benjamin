#include "kd_t_model.h"
#include "tree/context-dep.h"
#include <QDebug>

using namespace kaldi;

void KdTransitionModel::ComputeDerived()
{
    state2id_.resize(tuples_.size()+2);  // indexed by transition-state, which
    // is one based, but also an entry for one past end of list.

    int32 cur_transition_id = 1;
    num_pdfs_ = 0;
    for (int32 tstate = 1;
         tstate <= static_cast<int32>(tuples_.size()+1);  // not a typo.
         tstate++) {
        state2id_[tstate] = cur_transition_id;
        if (static_cast<size_t>(tstate) <= tuples_.size()) {
            int32 phone = tuples_[tstate-1].phone,
                    hmm_state = tuples_[tstate-1].hmm_state,
                    forward_pdf = tuples_[tstate-1].forward_pdf,
                    self_loop_pdf = tuples_[tstate-1].self_loop_pdf;
            num_pdfs_ = std::max(num_pdfs_, 1 + forward_pdf);
            num_pdfs_ = std::max(num_pdfs_, 1 + self_loop_pdf);
            const HmmTopology::HmmState &state = topo_.TopologyForPhone(phone)[hmm_state];
            int32 my_num_ids = static_cast<int32>(state.transitions.size());
            cur_transition_id += my_num_ids;  // # trans out of this state.
        }
    }

    id2state_.resize(cur_transition_id);   // cur_transition_id is #transition-ids+1.
    id2pdf.resize(cur_transition_id);
    for (int32 tstate = 1; tstate <= static_cast<int32>(tuples_.size()); tstate++) {
        for (int32 tid = state2id_[tstate]; tid < state2id_[tstate+1]; tid++) {
            id2state_[tid] = tstate;
            if (IsSelfLoop(tid))
                id2pdf[tid] = tuples_[tstate-1].self_loop_pdf;
            else
                id2pdf[tid] = tuples_[tstate-1].forward_pdf;
        }
    }

    // The following statements put copies a large number in the region of memory
    // past the end of the id2pdf_id_ array, while leaving the array as it was
    // before.  The goal of this is to speed up decoding by disabling a check
    // inside TransitionIdToPdf() that the transition-id was within the correct
    // range.
    int32 num_big_numbers = std::min<int32>(2000, cur_transition_id);
    id2pdf.resize(cur_transition_id + num_big_numbers,
                  std::numeric_limits<int32>::max());
    id2pdf.resize(cur_transition_id);
}

void KdTransitionModel::InitializeProbs() {
    log_probs_.Resize(NumTransitionIds()+1);  // one-based array, zeroth element empty.
    for (int32 trans_id = 1; trans_id <= NumTransitionIds(); trans_id++) {
        int32 trans_state = id2state_[trans_id];
        int32 trans_index = trans_id - state2id_[trans_state];
        const Tuple &tuple = tuples_[trans_state-1];
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

void KdTransitionModel::Check() const {
    KALDI_ASSERT(NumTransitionIds() != 0 && NumTransitionStates() != 0);
    {
        int32 sum = 0;
        for (int32 ts = 1; ts <= NumTransitionStates(); ts++) sum += NumTransitionIndices(ts);
        KALDI_ASSERT(sum == NumTransitionIds());
    }
    for (int32 tid = 1; tid <= NumTransitionIds(); tid++) {
        int32 tstate = TransitionIdToTransitionState(tid),
                index = TransitionIdToTransitionIndex(tid);
        KALDI_ASSERT(tstate > 0 && tstate <=NumTransitionStates() && index >= 0);
        KALDI_ASSERT(tid == PairToTransitionId(tstate, index));
        int32 phone = TransitionStateToPhone(tstate),
                hmm_state = TransitionStateToHmmState(tstate),
                forward_pdf = TransitionStateToForwardPdf(tstate),
                self_loop_pdf = TransitionStateToSelfLoopPdf(tstate);
        KALDI_ASSERT(tstate == TupleToTransitionState(phone, hmm_state, forward_pdf, self_loop_pdf));
        KALDI_ASSERT(log_probs_(tid) <= 0.0 && log_probs_(tid) - log_probs_(tid) == 0.0);
        // checking finite and non-positive (and not out-of-bounds).
    }
}

int32 KdTransitionModel::TupleToTransitionState(int32 phone, int32 hmm_state, int32 pdf, int32 self_loop_pdf) const {
    Tuple tuple(phone, hmm_state, pdf, self_loop_pdf);
    // Note: if this ever gets too expensive, which is unlikely, we can refactor
    // this code to sort first on pdf, and then index on pdf, so those
    // that have the same pdf are in a contiguous range.
    std::vector<Tuple>::const_iterator iter =
            std::lower_bound(tuples_.begin(), tuples_.end(), tuple);
    if (iter == tuples_.end() || !(*iter == tuple)) {
        KALDI_ERR << "KdTransitionModel::TupleToTransitionState, tuple not found."
                  << " (incompatible tree and model?)";
    }
    // tuples_ is indexed by transition_state-1, so add one.
    return static_cast<int32>((iter - tuples_.begin())) + 1;
}

int32 KdTransitionModel::NumTransitionIndices(int32 trans_state) const {
    KALDI_ASSERT(static_cast<size_t>(trans_state) <= tuples_.size());
    return static_cast<int32>(state2id_[trans_state+1]-state2id_[trans_state]);
}

int32 KdTransitionModel::TransitionIdToTransitionState(int32 trans_id) const {
    KALDI_ASSERT(trans_id != 0 &&  static_cast<size_t>(trans_id) < id2state_.size());
    return id2state_[trans_id];
}

int32 KdTransitionModel::TransitionIdToTransitionIndex(int32 trans_id) const {
    KALDI_ASSERT(trans_id != 0 && static_cast<size_t>(trans_id) < id2state_.size());
    return trans_id - state2id_[id2state_[trans_id]];
}

int32 KdTransitionModel::TransitionStateToPhone(int32 trans_state) const {
    KALDI_ASSERT(static_cast<size_t>(trans_state) <= tuples_.size());
    return tuples_[trans_state-1].phone;
}

int32 KdTransitionModel::TransitionStateToForwardPdf(int32 trans_state) const {
    KALDI_ASSERT(static_cast<size_t>(trans_state) <= tuples_.size());
    return tuples_[trans_state-1].forward_pdf;
}

int32 KdTransitionModel::TransitionStateToForwardPdfClass(
        int32 trans_state) const {
    KALDI_ASSERT(static_cast<size_t>(trans_state) <= tuples_.size());
    const Tuple &t = tuples_[trans_state-1];
    const HmmTopology::TopologyEntry &entry = topo_.TopologyForPhone(t.phone);
    KALDI_ASSERT(static_cast<size_t>(t.hmm_state) < entry.size());
    return entry[t.hmm_state].forward_pdf_class;
}


int32 KdTransitionModel::TransitionStateToSelfLoopPdfClass(
        int32 trans_state) const {
    KALDI_ASSERT(static_cast<size_t>(trans_state) <= tuples_.size());
    const Tuple &t = tuples_[trans_state-1];
    const HmmTopology::TopologyEntry &entry = topo_.TopologyForPhone(t.phone);
    KALDI_ASSERT(static_cast<size_t>(t.hmm_state) < entry.size());
    return entry[t.hmm_state].self_loop_pdf_class;
}


int32 KdTransitionModel::TransitionStateToSelfLoopPdf(int32 trans_state) const {
    KALDI_ASSERT(static_cast<size_t>(trans_state) <= tuples_.size());
    return tuples_[trans_state-1].self_loop_pdf;
}

int32 KdTransitionModel::TransitionStateToHmmState(int32 trans_state) const {
    KALDI_ASSERT(static_cast<size_t>(trans_state) <= tuples_.size());
    return tuples_[trans_state-1].hmm_state;
}

int32 KdTransitionModel::PairToTransitionId(int32 trans_state, int32 trans_index) const {
    KALDI_ASSERT(static_cast<size_t>(trans_state) <= tuples_.size());
    KALDI_ASSERT(trans_index < state2id_[trans_state+1] - state2id_[trans_state]);
    return state2id_[trans_state] + trans_index;
}

int32 KdTransitionModel::NumPhones() const {
    int32 num_trans_state = tuples_.size();
    int32 max_phone_id = 0;
    for (int32 i = 0; i < num_trans_state; ++i) {
        if (tuples_[i].phone > max_phone_id)
            max_phone_id = tuples_[i].phone;
    }
    return max_phone_id;
}


bool KdTransitionModel::IsFinal(int32 trans_id) const {
    KALDI_ASSERT(static_cast<size_t>(trans_id) < id2state_.size());
    int32 trans_state = id2state_[trans_id];
    int32 trans_index = trans_id - state2id_[trans_state];
    const Tuple &tuple = tuples_[trans_state-1];
    const HmmTopology::TopologyEntry &entry = topo_.TopologyForPhone(tuple.phone);
    KALDI_ASSERT(static_cast<size_t>(tuple.hmm_state) < entry.size());
    KALDI_ASSERT(static_cast<size_t>(tuple.hmm_state) < entry.size());
    KALDI_ASSERT(static_cast<size_t>(trans_index) <
                 entry[tuple.hmm_state].transitions.size());
    // return true if the transition goes to the final state of the
    // topology entry.
    return (entry[tuple.hmm_state].transitions[trans_index].first + 1 ==
            static_cast<int32>(entry.size()));
}



int32 KdTransitionModel::SelfLoopOf(int32 trans_state) const {  // returns the self-loop transition-id,
    KALDI_ASSERT(static_cast<size_t>(trans_state-1) < tuples_.size());
    const Tuple &tuple = tuples_[trans_state-1];
    // or zero if does not exist.
    int32 phone = tuple.phone, hmm_state = tuple.hmm_state;
    const HmmTopology::TopologyEntry &entry = topo_.TopologyForPhone(phone);
    KALDI_ASSERT(static_cast<size_t>(hmm_state) < entry.size());
    for (int32 trans_index = 0;
         trans_index < static_cast<int32>(entry[hmm_state].transitions.size());
         trans_index++)
        if (entry[hmm_state].transitions[trans_index].first == hmm_state)
            return PairToTransitionId(trans_state, trans_index);
    return 0;  // invalid transition id.
}

void KdTransitionModel::Read(std::istream &is, bool binary) {
    ExpectToken(is, binary, "<TransitionModel>");
    topo_.Read(is, binary);
    std::string token;
    ReadToken(is, binary, &token);
    int32 size;
    ReadBasicType(is, binary, &size);
    tuples_.resize(size);
    for (int32 i = 0; i < size; i++) {
        ReadBasicType(is, binary, &(tuples_[i].phone));
        ReadBasicType(is, binary, &(tuples_[i].hmm_state));
        ReadBasicType(is, binary, &(tuples_[i].forward_pdf));
        if (token == "<Tuples>")
            ReadBasicType(is, binary, &(tuples_[i].self_loop_pdf));
        else if (token == "<Triples>")
            tuples_[i].self_loop_pdf = tuples_[i].forward_pdf;
    }
    ReadToken(is, binary, &token);
    KALDI_ASSERT(token == "</Triples>" || token == "</Tuples>");
    ComputeDerived();
    ExpectToken(is, binary, "<LogProbs>");
    log_probs_.Read(is, binary);
    ExpectToken(is, binary, "</LogProbs>");
    ExpectToken(is, binary, "</TransitionModel>");
    Check();
}

int32 KdTransitionModel::TransitionIdToPhone(int32 trans_id) const {
    KALDI_ASSERT(trans_id != 0 && static_cast<size_t>(trans_id) < id2state_.size());
    int32 trans_state = id2state_[trans_id];
    return tuples_[trans_state-1].phone;
}

int32 KdTransitionModel::TransitionIdToPdfClass(int32 trans_id) const {
    KALDI_ASSERT(trans_id != 0 && static_cast<size_t>(trans_id) < id2state_.size());
    int32 trans_state = id2state_[trans_id];

    const Tuple &t = tuples_[trans_state-1];
    const HmmTopology::TopologyEntry &entry = topo_.TopologyForPhone(t.phone);
    KALDI_ASSERT(static_cast<size_t>(t.hmm_state) < entry.size());
    if (IsSelfLoop(trans_id))
        return entry[t.hmm_state].self_loop_pdf_class;
    else
        return entry[t.hmm_state].forward_pdf_class;
}


int32 KdTransitionModel::TransitionIdToHmmState(int32 trans_id) const {
    KALDI_ASSERT(trans_id != 0 && static_cast<size_t>(trans_id) < id2state_.size());
    int32 trans_state = id2state_[trans_id];
    const Tuple &t = tuples_[trans_state-1];
    return t.hmm_state;
}

bool GetPdfsForPhones(const KdTransitionModel &trans_model,
                      const std::vector<int32> &phones,
                      std::vector<int32> *pdfs) {
    KALDI_ASSERT(IsSortedAndUniq(phones));
    KALDI_ASSERT(pdfs != NULL);
    pdfs->clear();
    for (int32 tstate = 1; tstate <= trans_model.NumTransitionStates(); tstate++) {
        if (std::binary_search(phones.begin(), phones.end(),
                               trans_model.TransitionStateToPhone(tstate))) {
            pdfs->push_back(trans_model.TransitionStateToForwardPdf(tstate));
            pdfs->push_back(trans_model.TransitionStateToSelfLoopPdf(tstate));
        }
    }
    SortAndUniq(pdfs);

    for (int32 tstate = 1; tstate <= trans_model.NumTransitionStates(); tstate++)
        if ((std::binary_search(pdfs->begin(), pdfs->end(),
                                trans_model.TransitionStateToForwardPdf(tstate)) ||
             std::binary_search(pdfs->begin(), pdfs->end(),
                                trans_model.TransitionStateToSelfLoopPdf(tstate)))
                && !std::binary_search(phones.begin(), phones.end(),
                                       trans_model.TransitionStateToPhone(tstate)))
            return false;
    return true;
}

bool GetPhonesForPdfs(const KdTransitionModel &trans_model,
                      const std::vector<int32> &pdfs,
                      std::vector<int32> *phones) {
    KALDI_ASSERT(IsSortedAndUniq(pdfs));
    KALDI_ASSERT(phones != NULL);
    phones->clear();
    for (int32 tstate = 1; tstate <= trans_model.NumTransitionStates(); tstate++) {
        if (std::binary_search(pdfs.begin(), pdfs.end(),
                               trans_model.TransitionStateToForwardPdf(tstate)) ||
                std::binary_search(pdfs.begin(), pdfs.end(),
                                   trans_model.TransitionStateToSelfLoopPdf(tstate)))
            phones->push_back(trans_model.TransitionStateToPhone(tstate));
    }
    SortAndUniq(phones);

    for (int32 tstate = 1; tstate <= trans_model.NumTransitionStates(); tstate++)
        if (std::binary_search(phones->begin(), phones->end(),
                               trans_model.TransitionStateToPhone(tstate))
                && !(std::binary_search(pdfs.begin(), pdfs.end(),
                                        trans_model.TransitionStateToForwardPdf(tstate)) &&
                     std::binary_search(pdfs.begin(), pdfs.end(),
                                        trans_model.TransitionStateToSelfLoopPdf(tstate))) )
            return false;
    return true;
}

bool KdTransitionModel::IsSelfLoop(int32 trans_id) const {
    KALDI_ASSERT(static_cast<size_t>(trans_id) < id2state_.size());
    int32 trans_state = id2state_[trans_id];
    int32 trans_index = trans_id - state2id_[trans_state];
    const Tuple &tuple = tuples_[trans_state-1];
    int32 phone = tuple.phone, hmm_state = tuple.hmm_state;
    const HmmTopology::TopologyEntry &entry = topo_.TopologyForPhone(phone);
    KALDI_ASSERT(static_cast<size_t>(hmm_state) < entry.size());
    return (static_cast<size_t>(trans_index) < entry[hmm_state].transitions.size()
            && entry[hmm_state].transitions[trans_index].first == hmm_state);
}
