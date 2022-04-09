#include "kd_lattice_det.h"

using namespace kaldi;

void KdLatDet::Output(KdCompactLattice *ofst, bool destroy)
{
    KALDI_ASSERT(determinized_);
    StateId nStates = static_cast<StateId>(output_states_.size());
    if( destroy)
        FreeMostMemory();
    ofst->DeleteStates();
    ofst->SetStart(KD_INVALID_STATE);
    if( nStates==0) {
        return;
    }
    for (StateId s = 0;s < nStates;s++) {
        KdStateId news = ofst->AddState();
        KALDI_ASSERT(news==s);
    }
    ofst->SetStart(0);
    // now process transitions.
    for (StateId this_state_id = 0; this_state_id < nStates; this_state_id++) {
        OutputState &this_state = *(output_states_[this_state_id]);
        std::vector<TempArc> &this_vec(this_state.arcs);
        typename std::vector<TempArc>::const_iterator iter = this_vec.begin(), end = this_vec.end();

        for (;iter!=end; ++iter)
        {
            const TempArc &temp_arc(*iter);
            KdCLatArc new_arc;
            std::vector<Label> olabel_seq;
            repository_.ConvertToVector(temp_arc.string, &olabel_seq);
            KdCLatWeight weight(temp_arc.weight, olabel_seq);
            if( temp_arc.nextstate==KD_INVALID_STATE )
            {  // is really final weight.
                ofst->SetFinal(this_state_id, weight);
            }
            else
            {  // is really an arc.
                new_arc.nextstate = temp_arc.nextstate;
                new_arc.ilabel = temp_arc.ilabel;
                new_arc.olabel = temp_arc.ilabel;  // acceptor.  input==output.
                new_arc.weight = weight;  // includes string and weight.
                ofst->AddArc(this_state_id, new_arc);
            }
        }
        // Free up memory.  Do this inside the loop as ofst is also allocating memory,
        // and we want to reduce the maximum amount ever allocated.
        if( destroy)
        {
            std::vector<TempArc> temp;
            temp.swap(this_vec);
        }
    }
    if( destroy) {
        FreeOutputStates();
        repository_.Destroy();
    }
}

void KdLatDet::Output(KdLattice *ofst, bool destroy)
{
    // Outputs to standard fst.
    KdStateId nStates = static_cast<KdStateId>(output_states_.size());
    ofst->DeleteStates();
    if( nStates==0) {
        ofst->SetStart(KD_INVALID_STATE);
        return;
    }
    if( destroy)
        FreeMostMemory();
    // Add basic states-- but we will add extra ones to account for strings on output.
    for (KdStateId s = 0; s< nStates;s++) {
        KdStateId news = ofst->AddState();
        KALDI_ASSERT(news==s);
    }
    ofst->SetStart(0);
    for (KdStateId this_state_id = 0; this_state_id < nStates; this_state_id++) {
        OutputState &this_state = *(output_states_[this_state_id]);
        std::vector<TempArc> &this_vec(this_state.arcs);

        typename std::vector<TempArc>::const_iterator iter = this_vec.begin(), end = this_vec.end();
        for (; iter!=end; ++iter) {
            const TempArc &temp_arc(*iter);
            std::vector<Label> seq;
            repository_.ConvertToVector(temp_arc.string, &seq);

            if( temp_arc.nextstate==KD_INVALID_STATE) {  // Really a final weight.
                // Make a sequence of states going to a final state, with the strings
                // as labels.  Put the weight on the first arc.
                KdStateId cur_state = this_state_id;
                for (size_t i = 0; i < seq.size(); i++) {
                    KdStateId next_state = ofst->AddState();
                    KdLatticeArc arc;
                    arc.nextstate = next_state;
                    arc.weight = (i==0 ? temp_arc.weight : KdLatticeWeight::One());
                    arc.ilabel = 0;  // epsilon.
                    arc.olabel = seq[i];
                    ofst->AddArc(cur_state, arc);
                    cur_state = next_state;
                }
                ofst->SetFinal(cur_state, (seq.size()==0 ? temp_arc.weight : KdLatticeWeight::One()));
            } else {  // Really an arc.
                KdStateId cur_state = this_state_id;
                // Have to be careful with this integer comparison (i+1 < seq.size()) because unsigned.
                // i < seq.size()-1 could fail for zero-length sequences.
                for (size_t i = 0; i+1 < seq.size();i++) {
                    // for all but the last element of seq, create new state.
                    KdStateId next_state = ofst->AddState();
                    KdLatticeArc arc;
                    arc.nextstate = next_state;
                    arc.weight = (i==0 ? temp_arc.weight : KdLatticeWeight::One());
                    arc.ilabel = (i==0 ? temp_arc.ilabel : 0);  // put ilabel on first element of seq.
                    arc.olabel = seq[i];
                    ofst->AddArc(cur_state, arc);
                    cur_state = next_state;
                }
                // Add the final arc in the sequence.
                KdLatticeArc arc;
                arc.nextstate = temp_arc.nextstate;
                arc.weight = (seq.size() <= 1 ? temp_arc.weight : KdLatticeWeight::One());
                arc.ilabel = (seq.size() <= 1 ? temp_arc.ilabel : 0);
                arc.olabel = (seq.size() > 0 ? seq.back() : 0);
                ofst->AddArc(cur_state, arc);
            }
        }
        // Free up memory.  Do this inside the loop as ofst is also allocating memory
        if( destroy) { std::vector<TempArc> temp; temp.swap(this_vec); }
    }
    if( destroy) {
        FreeOutputStates();
        repository_.Destroy();
    }
}

KdLatDet::KdLatDet(KdLattice &ifst, double beam,
                   KdDetOpt opts):
    num_arcs_(0), num_elems_(0), ifst_(ifst.Copy()), beam_(beam), opts_(opts),
    equal_(opts_.delta), determinized_(false),
    minimal_hash_(3, hasher_, equal_), initial_hash_(3, hasher_, equal_)
{
    // this algorithm won't work correctly otherwise.
}

void KdLatDet::FreeOutputStates()
{
    for (size_t i = 0; i < output_states_.size(); i++)
        delete output_states_[i];
    std::vector<OutputState*> temp;
    temp.swap(output_states_);
}

void KdLatDet::FreeMostMemory()
{
    if( ifst_)
    {
        delete ifst_;
        ifst_ = NULL;
    }
    { MinimalSubsetHash tmp; tmp.swap(minimal_hash_); }

    for (size_t i = 0; i < output_states_.size(); i++) {
        std::vector<Element> empty_subset;
        empty_subset.swap(output_states_[i]->minimal_subset);
    }

    for (typename InitialSubsetHash::iterator iter = initial_hash_.begin();
         iter!=initial_hash_.end(); ++iter)
        delete iter->first;
    { InitialSubsetHash tmp; tmp.swap(initial_hash_); }
    for (size_t i = 0; i < output_states_.size(); i++)
    {
        std::vector<Element> tmp;
        tmp.swap(output_states_[i]->minimal_subset);
    }
    { std::vector<char> tmp;  tmp.swap(isymbol_or_final_); }
    { // Free up the queue.  I'm not sure how to make sure all
        // the memory is really freed (no swap() function)... doesn't really
        // matter much though.
        while (!queue_.empty())
        {
            Task *t = queue_.top();
            delete t;
            queue_.pop();
        }
    }
    std::vector<std::pair<Label, Element> > tmp;
    tmp.swap(all_elems_tmp_);
}

KdLatDet::~KdLatDet()
{
    FreeMostMemory();
    FreeOutputStates();
    // rest is deleted by destructors.
}

void KdLatDet::RebuildRepository()
{ // rebuild the string repository,
    // freeing stuff we don't need.. we call this when memory usage
    // passes a supplied threshold.  We need to accumulate all the
    // strings we need the repository to "remember", then tell it
    // to clean the repository.
    std::vector<StringId> needed_strings;
    for (size_t i = 0; i < output_states_.size(); i++) {
        AddStrings(output_states_[i]->minimal_subset, &needed_strings);
        for (size_t j = 0; j < output_states_[i]->arcs.size(); j++)
            needed_strings.push_back(output_states_[i]->arcs[j].string);
    }

    { // the queue doesn't allow us access to the underlying vector,
        // so we have to resort to a temporary collection.
        std::vector<Task*> tasks;
        while (!queue_.empty()) {
            Task *task = queue_.top();
            queue_.pop();
            tasks.push_back(task);
            AddStrings(task->subset, &needed_strings);
        }
        for (size_t i = 0; i < tasks.size(); i++)
            queue_.push(tasks[i]);
    }

    // the following loop covers strings present in initial_hash_.
    for (typename InitialSubsetHash::const_iterator
         iter = initial_hash_.begin();
         iter!=initial_hash_.end(); ++iter) {
        const std::vector<Element> &vec = *(iter->first);
        Element elem = iter->second;
        AddStrings(vec, &needed_strings);
        needed_strings.push_back(elem.string);
    }
    std::sort(needed_strings.begin(), needed_strings.end());
    needed_strings.erase(std::unique(needed_strings.begin(),
                                     needed_strings.end()),
                         needed_strings.end()); // uniq the strings.
    KALDI_LOG << "Rebuilding repository.";

    repository_.Rebuild(needed_strings);
}

bool KdLatDet::CheckMemoryUsage()
{
    int32 repo_size = repository_.MemSize(),
            arcs_size = num_arcs_ * sizeof(TempArc),
            elems_size = num_elems_ * sizeof(Element),
            total_size = repo_size + arcs_size + elems_size;
    if( opts_.max_mem > 0 && total_size > opts_.max_mem) { // We passed the memory threshold.
        // This is usually due to the repository getting large, so we
        // clean this out.
        RebuildRepository();
        int32 new_repo_size = repository_.MemSize(),
                new_total_size = new_repo_size + arcs_size + elems_size;

        KALDI_VLOG(2) << "Rebuilt repository in determinize-lattice: repository shrank from "
                      << repo_size << " to " << new_repo_size << " bytes (approximately)";

        if( new_total_size > static_cast<int32>(opts_.max_mem * 0.8)) {
            // Rebuilding didn't help enough-- we need a margin to stop
            // having to rebuild too often.  We'll just return to the user at
            // this point, with a partial lattice that's pruned tighter than
            // the specified beam.  Here we figure out what the effective
            // beam was.
            double effective_beam = beam_;
            if( !queue_.empty()) { // Note: queue should probably not be empty; we're
                // just being paranoid here.
                Task *task = queue_.top();
                double total_weight = backward_costs_[ifst_->Start()]; // best weight of FST.
                effective_beam = task->priority_cost - total_weight;
            }
            KALDI_WARN << "Did not reach requested beam in determinize-lattice: "
                       << "size exceeds maximum " << opts_.max_mem
                       << " bytes; (repo,arcs,elems) = (" << repo_size << ","
                       << arcs_size << "," << elems_size
                       << "), after rebuilding, repo size was " << new_repo_size
                       << ", effective beam was " << effective_beam
                       << " vs. requested beam " << beam_;
            return false;
        }
    }
    return true;
}

bool KdLatDet::Determinize(double *effective_beam)
{
    KALDI_ASSERT(!determinized_);
    // This determinizes the input fst but leaves it in the "special format"
    // in "output_arcs_".  Must be called after Initialize().  To get the
    // output, call one of the Output routines.

    InitializeDeterminization(); // some start-up tasks.
    while (!queue_.empty()) {
        Task *task = queue_.top();
        // Note: the queue contains only tasks that are "within the beam".
        // We also have to check whether we have reached one of the user-specified
        // maximums, of estimated memory, arcs, or states.  The condition for
        // ending is:
        // num-states is more than user specified, OR
        // num-arcs is more than user specified, OR
        // memory passed a user-specified threshold and cleanup failed
        //  to get it below that threshold.
        size_t num_states = output_states_.size();
        if( (opts_.max_states > 0 && num_states > opts_.max_states) ||
                (opts_.max_arcs > 0 && num_arcs_ > opts_.max_arcs) ||
                (num_states % 10==0 && !CheckMemoryUsage())) { // note: at some point
            // it was num_states % 100, not num_states % 10, but I encountered an example
            // where memory was exhausted before we reached state #100.
            KALDI_VLOG(1) << "Lattice determinization terminated but not "
                          << " because of lattice-beam.  (#states, #arcs) is ( "
                          << output_states_.size() << ", " << num_arcs_
                          << " ), versus limits ( " << opts_.max_states << ", "
                          << opts_.max_arcs << " ) (else, may be memory limit).";
            break;
            // we terminate the determinization here-- whatever we already expanded is
            // what we'll return...  because we expanded stuff in order of total
            // (forward-backward) weight, the stuff we returned first is the most
            // important.
        }
        queue_.pop();
        ProcessTransition(task->state, task->label, &(task->subset));
        delete task;
    }
    determinized_ = true;
    if( effective_beam!=NULL) {
        if( queue_.empty()) *effective_beam = beam_;
        else
            *effective_beam = queue_.top()->priority_cost -
                backward_costs_[ifst_->Start()];
    }
    return (queue_.empty()); // return success if queue was empty, i.e. we processed
    // all tasks and did not break out of the loop early due to reaching a memory,
    // arc or state limit.
}

void KdLatDet::ConvertToMinimal(std::vector<Element> *subset)
{
    KALDI_ASSERT(!subset->empty());
    typename std::vector<Element>::iterator cur_in = subset->begin(),
            cur_out = subset->begin(), end = subset->end();
    while (cur_in!=end) {
        if(IsIsymbolOrFinal(cur_in->state)) {  // keep it...
            *cur_out = *cur_in;
            cur_out++;
        }
        cur_in++;
    }
    subset->resize(cur_out - subset->begin());
}

KdStateId KdLatDet::MinimalToStateId(const std::vector<Element> &subset,
                               const double forward_cost)
{
    typename MinimalSubsetHash::const_iterator iter
            = minimal_hash_.find(&subset);
    if( iter!=minimal_hash_.end()) { // Found a matching subset.
        KdStateId state_id = iter->second;
        const OutputState &state = *(output_states_[state_id]);
        // Below is just a check that the algorithm is working...
        if( forward_cost < state.forward_cost - 0.1) {
            // for large weights, this check could fail due to roundoff.
            KALDI_WARN << "New cost is less (check the difference is small) "
                       << forward_cost << ", "
                       << state.forward_cost;
        }
        return state_id;
    }
    KdStateId state_id = static_cast<KdStateId>(output_states_.size());
    OutputState *new_state = new OutputState(subset, forward_cost);
    minimal_hash_[&(new_state->minimal_subset)] = state_id;
    output_states_.push_back(new_state);
    num_elems_ += subset.size();
    // Note: in the previous algorithm, we pushed the new state-id onto the queue
    // at this point.  Here, the queue happens elsewhere, and we directly process
    // the state (which result in stuff getting added to the queue).
    ProcessFinal(state_id); // will work out the final-prob.
    ProcessTransitions(state_id); // will process transitions and add stuff to the queue.
    return state_id;
}

KdStateId KdLatDet::InitialToStateId(const std::vector<Element> &subset_in,
                               double forward_cost,
                               KdLatticeWeight *remaining_weight,
                               StringId *common_prefix)
{
    typename InitialSubsetHash::const_iterator iter
            = initial_hash_.find(&subset_in);
    if( iter!=initial_hash_.end())
    { // Found a matching subset.
        Element elem = iter->second;
        *remaining_weight = elem.weight;
        *common_prefix = elem.string;
        if(  elem.weight.isZero() )
            KALDI_WARN << "Zero weight!";
        return elem.state;
    }
    // else no matching subset-- have to work it out.
    std::vector<Element> subset(subset_in);
    // Follow through epsilons.  Will add no duplicate states.  note: after
    // EpsilonClosure, it is the same as "canonical" subset, except not
    // normalized (actually we never compute the normalized canonical subset,
    // only the normalized minimal one).
    EpsilonClosure(&subset); // follow epsilons.
    ConvertToMinimal(&subset); // remove all but emitting and final states.

    Element elem; // will be used to store remaining weight and string, and
    // KdStateId, in initial_hash_;
    NormalizeSubset(&subset, &elem.weight, &elem.string); // normalize subset; put
    // common string and weight in "elem".  The subset is now a minimal,
    // normalized subset.

    forward_cost += elem.weight.getCost();
    KdStateId ans = MinimalToStateId(subset, forward_cost);
    *remaining_weight = elem.weight;
    *common_prefix = elem.string;
    if( elem.weight.isZero() )
    {
        qDebug() << "Zero weight!";
    }

    // Before returning "ans", add the initial subset to the hash,
    // so that we can bypass the epsilon-closure etc., next time
    // we process the same initial subset.
    std::vector<Element> *initial_subset_ptr = new std::vector<Element>(subset_in);
    elem.state = ans;
    initial_hash_[initial_subset_ptr] = elem;
    num_elems_ += initial_subset_ptr->size(); // keep track of memory usage.
    return ans;
}

int KdLatDet::CompareDet(const KdLatticeWeight &a_w, StringId a_str,
                   const KdLatticeWeight &b_w, StringId b_str)
{
    int weight_comp = Compare(a_w, b_w);
    if( weight_comp!=0) return weight_comp;
    // now comparing strings.
    if( a_str==b_str) return 0;
    std::vector<int> a_vec, b_vec;
    repository_.ConvertToVector(a_str, &a_vec);
    repository_.ConvertToVector(b_str, &b_vec);
    // First compare their lengths.
    int a_len = a_vec.size(), b_len = b_vec.size();
    // use opposite order on the string lengths (c.f. Compare in
    // lattice-weight.h)
    if( a_len > b_len) return -1;
    else if( a_len < b_len) return 1;
    for(int i = 0; i < a_len; i++) {
        if( a_vec[i] < b_vec[i]) return -1;
        else if( a_vec[i] > b_vec[i]) return 1;
    }
    KALDI_ASSERT(0); // because we checked if a_str==b_str above, shouldn't reach here
    return 0;
}

void KdLatDet::EpsilonClosure(std::vector<Element> *subset) {
    // at input, subset must have only one example of each StateId.  [will still
    // be so at output].  This function follows input-epsilons, and augments the
    // subset accordingly.

    std::priority_queue<Element, std::vector<Element>, std::greater<Element> > queue;
    unordered_map<InputStateId, Element> cur_subset;
    typedef typename unordered_map<InputStateId, Element>::iterator MapIter;
    typedef typename std::vector<Element>::const_iterator VecIter;

    for (VecIter iter = subset->begin(); iter!=subset->end(); ++iter) {
        queue.push(*iter);
        cur_subset[iter->state] = *iter;
    }

    // find whether input fst is known to be sorted on input label.
    bool sorted = ((ifst_->Properties(fst::kILabelSorted, false) & fst::kILabelSorted)!=0);
    bool replaced_elems = false; // relates to an optimization, see below.
    int counter = 0; // stops infinite loops here for non-lattice-determinizable input
    // (e.g. input with negative-cost epsilon loops); useful in testing.
    while (queue.size()!=0) {
        Element elem = queue.top();
        queue.pop();

        // The next if-statement is a kind of optimization.  It's to prevent us
        // unnecessarily repeating the processing of a state.  "cur_subset" always
        // contains only one Element with a particular state.  The issue is that
        // whenever we modify the Element corresponding to that state in "cur_subset",
        // both the new (optimal) and old (less-optimal) Element will still be in
        // "queue".  The next if-statement stops us from wasting compute by
        // processing the old Element.
        if( replaced_elems && cur_subset[elem.state]!=elem)
            continue;
        if( opts_.max_loop > 0 && counter++ > opts_.max_loop) {
            qDebug() << "Lattice determinization aborted since looped more than "
                      << opts_.max_loop << " times during epsilon closure.";
        }
        for (fst::ArcIterator<fst::ExpandedFst<KdLatticeArc> > aiter(*ifst_, elem.state); !aiter.Done(); aiter.Next())
        {
            KdLatticeArc arc = aiter.Value();
            if( sorted && arc.ilabel!=0) break;  // Break from the loop: due to sorting there will be no
            // more transitions with epsilons as input labels.
            if( arc.ilabel==0
                    && !(arc.weight.isZero()) )
            {  // Epsilon transition.
                Element next_elem;
                next_elem.state = arc.nextstate;
                next_elem.weight = Times(elem.weight, arc.weight);
                // next_elem.string is not set up yet... create it only
                // when we know we need it (this is an optimization)

                MapIter iter = cur_subset.find(next_elem.state);
                if( iter==cur_subset.end())
                {
                    // was no such StateId: insert and add to queue.
                    next_elem.string = (arc.olabel==0 ? elem.string :
                                                          repository_.Successor(elem.string, arc.olabel));
                    cur_subset[next_elem.state] = next_elem;
                    queue.push(next_elem);
                }
                else
                {
                    // was not inserted because one already there.  In normal
                    // determinization we'd add the weights.  Here, we find which one
                    // has the better weight, and keep its corresponding string.
                    int comp = Compare(next_elem.weight, iter->second.weight);
                    if( comp==0) { // A tie on weights.  This should be a rare case;
                        // we don't optimize for it.
                        next_elem.string = (arc.olabel==0 ? elem.string :
                                                              repository_.Successor(elem.string,
                                                                                    arc.olabel));
                        comp = CompareDet(next_elem.weight, next_elem.string,
                                       iter->second.weight, iter->second.string);
                    }
                    if(comp==1) { // next_elem is better, so use its (weight, string)
                        next_elem.string = (arc.olabel==0 ? elem.string :
                                                              repository_.Successor(elem.string, arc.olabel));
                        iter->second.string = next_elem.string;
                        iter->second.weight = next_elem.weight;
                        queue.push(next_elem);
                        replaced_elems = true;
                    }
                    // else it is the same or worse, so use original one.
                }
            }
        }
    }

    { // copy cur_subset to subset.
        subset->clear();
        subset->reserve(cur_subset.size());
        MapIter iter = cur_subset.begin(), end = cur_subset.end();
        for (; iter!=end; ++iter) subset->push_back(iter->second);
        // sort by state ID, because the subset hash function is order-dependent(see SubsetKey)
        std::sort(subset->begin(), subset->end());
    }
}

void KdLatDet::ProcessFinal(KdStateId output_state_id)
{
    OutputState &state = *(output_states_[output_state_id]);
    const std::vector<Element> &minimal_subset = state.minimal_subset;
    // processes final-weights for this subset.  state.minimal_subset_ may be
    // empty if the graphs is not connected/trimmed, I think, do don't check
    // that it's nonempty.
    StringId final_string = repository_.EmptyString();  // set it to keep the
    // compiler happy; if it doesn't get set in the loop, we won't use the value anyway.
    KdLatticeWeight final_weight = KdLatticeWeight::Zero();
    bool is_final = false;
    typename std::vector<Element>::const_iterator iter = minimal_subset.begin(), end = minimal_subset.end();
    for (; iter!=end; ++iter) {
        const Element &elem = *iter;
        KdLatticeWeight this_final_weight = Times(elem.weight, ifst_->Final(elem.state));
        StringId this_final_string = elem.string;
        if( this_final_weight!=KdLatticeWeight::Zero() &&
                (!is_final || CompareDet(this_final_weight, this_final_string,
                                      final_weight, final_string)==1)) { // the new
            // (weight, string) pair is more in semiring than our current
            // one.
            is_final = true;
            final_weight = this_final_weight;
            final_string = this_final_string;
        }
    }
    if( is_final &&
            final_weight.getCost() + state.forward_cost <= cutoff_) {
        // store final weights in TempArc structure, just like a transition.
        // Note: we only store the final-weight if it's inside the pruning beam, hence
        // the stuff with Compare.
        TempArc temp_arc;
        temp_arc.ilabel = 0;
        temp_arc.nextstate = KD_INVALID_STATE;  // special marker meaning "final weight".
        temp_arc.string = final_string;
        temp_arc.weight = final_weight;
        state.arcs.push_back(temp_arc);
        num_arcs_++;
    }
}

// NormalizeSubset normalizes the subset "elems" by
// removing any common string prefix (putting it in common_str),
// and dividing by the total weight (putting it in tot_weight).
void KdLatDet::NormalizeSubset(std::vector<Element> *elems,
                     KdLatticeWeight *tot_weight,
                     StringId *common_str)
{
    if(elems->empty()) { // just set common_str, tot_weight
        // to defaults and return...
        KALDI_WARN << "empty subset";
        *common_str = repository_.EmptyString();
        *tot_weight = KdLatticeWeight::Zero();
        return;
    }
    size_t size = elems->size();
    std::vector<int> common_prefix;
    repository_.ConvertToVector((*elems)[0].string, &common_prefix);
    KdLatticeWeight weight = (*elems)[0].weight;
    for(size_t i = 1; i < size; i++)
    {
        weight = Plus(weight, (*elems)[i].weight);
        repository_.ReduceToCommonPrefix((*elems)[i].string, &common_prefix);
    }
    KALDI_ASSERT(weight!=KdLatticeWeight::Zero()); // we made sure to ignore arcs with zero
    // weights on them, so we shouldn't have zero here.
    size_t prefix_len = common_prefix.size();
    for(size_t i = 0; i < size; i++)
    {
        (*elems)[i].weight = Divide((*elems)[i].weight, weight, fst::DIVIDE_LEFT);
        (*elems)[i].string =
                repository_.RemovePrefix((*elems)[i].string, prefix_len);
    }
    *common_str = repository_.ConvertFromVector(common_prefix);
    *tot_weight = weight;
}

// Take a subset of Elements that is sorted on state, and
// merge any Elements that have the same state (taking the best
// (weight, string) pair in the semiring).
void KdLatDet::MakeSubsetUnique(std::vector<Element> *subset)
{
    typedef typename std::vector<Element>::iterator IterType;

    // This KALDI_ASSERT is designed to fail (usually) if the subset is not sorted on
    // state.
    KALDI_ASSERT(subset->size() < 2 || (*subset)[0].state <= (*subset)[1].state);

    IterType cur_in = subset->begin(), cur_out = cur_in, end = subset->end();
    size_t num_out = 0;
    // Merge elements with same state-id
    while (cur_in!=end) {  // while we have more elements to process.
        // At this point, cur_out points to location of next place we want to put an element,
        // cur_in points to location of next element we want to process.
        if( cur_in!=cur_out) *cur_out = *cur_in;
        cur_in++;
        while (cur_in!=end && cur_in->state==cur_out->state) {
            if( CompareDet(cur_in->weight, cur_in->string,
                        cur_out->weight, cur_out->string)==1) {
                // if *cur_in > *cur_out in semiring, then take *cur_in.
                cur_out->string = cur_in->string;
                cur_out->weight = cur_in->weight;
            }
            cur_in++;
        }
        cur_out++;
        num_out++;
    }
    subset->resize(num_out);
}

void KdLatDet::ProcessTransition(KdStateId ostate_id, Label ilabel, std::vector<Element> *subset)
{

    double forward_cost = output_states_[ostate_id]->forward_cost;
    StringId common_str;
    KdLatticeWeight tot_weight;
    NormalizeSubset(subset, &tot_weight, &common_str);
    forward_cost += tot_weight.getCost();

    KdStateId nextstate;
    {
        KdLatticeWeight next_tot_weight;
        StringId next_common_str;
        nextstate = InitialToStateId(*subset,
                                     forward_cost,
                                     &next_tot_weight,
                                     &next_common_str);
        common_str = repository_.Concatenate(common_str, next_common_str);
        tot_weight = Times(tot_weight, next_tot_weight);
    }

    // Now add an arc to the next state (would have been created if necessary by
    // InitialToStateId).
    TempArc temp_arc;
    temp_arc.ilabel = ilabel;
    temp_arc.nextstate = nextstate;
    temp_arc.string = common_str;
    temp_arc.weight = tot_weight;
    output_states_[ostate_id]->arcs.push_back(temp_arc);  // record the arc.
    num_arcs_++;
}

void KdLatDet::ProcessTransitions(KdStateId output_state_id)
{
    const std::vector<Element> &minimal_subset = output_states_[output_state_id]->minimal_subset;
    // it's possible that minimal_subset could be empty if there are
    // unreachable parts of the graph, so don't check that it's nonempty.
    std::vector<std::pair<Label, Element> > &all_elems(all_elems_tmp_); // use class member
    // to avoid memory allocation/deallocation.
    {
        // Push back into "all_elems", elements corresponding to all
        // non-epsilon-input transitions out of all states in "minimal_subset".
        typename std::vector<Element>::const_iterator iter = minimal_subset.begin(), end = minimal_subset.end();
        for (;iter!=end; ++iter) {
            const Element &elem = *iter;
            for (fst::ArcIterator<fst::ExpandedFst<KdLatticeArc> > aiter(*ifst_, elem.state); ! aiter.Done(); aiter.Next()) {
                const KdLatticeArc &arc = aiter.Value();
                if( arc.ilabel!=0
                        && arc.weight!=KdLatticeWeight::Zero()) {  // Non-epsilon transition -- ignore epsilons here.
                    std::pair<Label, Element> this_pr;
                    this_pr.first = arc.ilabel;
                    Element &next_elem(this_pr.second);
                    next_elem.state = arc.nextstate;
                    next_elem.weight = Times(elem.weight, arc.weight);
                    if( arc.olabel==0) // output epsilon
                        next_elem.string = elem.string;
                    else
                        next_elem.string = repository_.Successor(elem.string, arc.olabel);
                    all_elems.push_back(this_pr);
                }
            }
        }
    }
    PairComparator pc;
    std::sort(all_elems.begin(), all_elems.end(), pc);
    // now sorted first on input label, then on state.
    typedef typename std::vector<std::pair<Label, Element> >::const_iterator PairIter;
    PairIter cur = all_elems.begin(), end = all_elems.end();
    while (cur!=end)
    {
        // The old code (non-pruned) called ProcessTransition; here, instead,
        // we'll put the calls into a priority queue.
        Task *task = new Task;
        // Process ranges that share the same input symbol.
        Label ilabel = cur->first;
        task->state = output_state_id;
        task->priority_cost = std::numeric_limits<double>::infinity();
        task->label = ilabel;
        while (cur!=end && cur->first==ilabel)
        {
            task->subset.push_back(cur->second);
            Element element = cur->second;
            // Note: we'll later include the term "forward_cost" in the
            // priority_cost.
            task->priority_cost = std::min(task->priority_cost,
                                           element.weight.getCost() +
                                           backward_costs_[element.state]);
            cur++;
        }

        // After the command below, the "priority_cost" is a value comparable to
        // the total-weight of the input FST, like a total-path weight... of
        // course, it will typically be less (in the semiring) than that.
        // note: we represent it just as a double.
        task->priority_cost += output_states_[output_state_id]->forward_cost;

        if( task->priority_cost > cutoff_) {
            // This task would never get done as it's past the pruning cutoff.
            delete task;
        } else {
            MakeSubsetUnique(&(task->subset)); // remove duplicate Elements with the same state.
            queue_.push(task); // Push the task onto the queue.  The queue keeps it
            // in prioritized order, so we always process the one with the "best"
            // weight (highest in the semiring).

            { // this is a check.
                double best_cost = backward_costs_[ifst_->Start()],
                        tolerance = 0.01 + 1.0e-04 * std::abs(best_cost);
                if( task->priority_cost < best_cost - tolerance) {
                    KALDI_WARN << "Cost below best cost was encountered:"
                               << task->priority_cost << " < " << best_cost;
                }
            }
        }
    }
    all_elems.clear(); // as it's a reference to a class variable; we want it to stay
    // empty.
}


bool KdLatDet::IsIsymbolOrFinal(InputStateId state)
{ // returns true if this state
    // of the input FST either is final or has an osymbol on an arc out of it.
    // Uses the vector isymbol_or_final_ as a cache for this info.
    KALDI_ASSERT(state >= 0);
    if( isymbol_or_final_.size() <= state)
        isymbol_or_final_.resize(state+1, static_cast<char>(OSF_UNKNOWN));
    if( isymbol_or_final_[state]==static_cast<char>(OSF_NO))
        return false;
    else if( isymbol_or_final_[state]==static_cast<char>(OSF_YES))
        return true;
    // else work it out...
    isymbol_or_final_[state] = static_cast<char>(OSF_NO);
    if( ifst_->Final(state)!=KdLatticeWeight::Zero())
        isymbol_or_final_[state] = static_cast<char>(OSF_YES);
    for (fst::ArcIterator<fst::ExpandedFst<KdLatticeArc> > aiter(*ifst_, state);
         !aiter.Done();
         aiter.Next())
    {
        const KdLatticeArc &arc = aiter.Value();
        if( arc.ilabel!=0 && arc.weight!=KdLatticeWeight::Zero()) {
            isymbol_or_final_[state] = static_cast<char>(OSF_YES);
            return true;
        }
    }
    return IsIsymbolOrFinal(state); // will only recurse once.
}

void KdLatDet::ComputeBackwardWeight()
{
    // Sets up the backward_costs_ array, and the cutoff_ variable.
    KALDI_ASSERT(beam_ > 0);

    // Only handle the toplogically sorted case.
    backward_costs_.resize(ifst_->NumStates());
    for (StateId s = ifst_->NumStates() - 1; s >= 0; s--)
    {
        double &cost = backward_costs_[s];
        cost = ifst_->Final(s).getCost();
        for (fst::ArcIterator<fst::ExpandedFst<KdLatticeArc> > aiter(*ifst_, s);
             !aiter.Done(); aiter.Next())
        {
            KdLatticeArc arc = aiter.Value();
            cost = std::min(cost,
                            arc.weight.getCost() + backward_costs_[arc.nextstate]);
        }
    }

    if( ifst_->Start()==KD_INVALID_STATE) return; // we'll be returning
    // an empty FST.

    double best_cost = backward_costs_[ifst_->Start()];
    if( best_cost==std::numeric_limits<double>::infinity())
        KALDI_WARN << "Total weight of input lattice is zero.";
    cutoff_ = best_cost + beam_;
}

void KdLatDet::InitializeDeterminization()
{
    // We insist that the input lattice be topologically sorted.  This is not a
    // fundamental limitation of the algorithm (which in principle should be
    // applicable to even cyclic FSTs), but it helps us more efficiently
    // compute the backward_costs_ array.  There may be some other reason we
    // require this, that escapes me at the moment.
    KALDI_ASSERT(ifst_->Properties(fst::kTopSorted, true)!=0);
    ComputeBackwardWeight();
#if !(__GNUC__==4 && __GNUC_MINOR__==0)
    if(ifst_->Properties(fst::kExpanded, false)!=0)
    { // if we know the number of
        // states in ifst_, it might be a bit more efficient
        // to pre-size the hashes so we're not constantly rebuilding them.
        StateId num_states = ifst_->NumStates();
        minimal_hash_.rehash(num_states/2 + 3);
        initial_hash_.rehash(num_states/2 + 3);
    }
#endif
    InputStateId start_id = ifst_->Start();
    if( start_id!=KD_INVALID_STATE) {
        /* Create determinized-state corresponding to the start state....
     Unlike all the other states, we don't "normalize" the representation
     of this determinized-state before we put it into minimal_hash_.  This is actually
     what we want, as otherwise we'd have problems dealing with any extra weight
     and string and might have to create a "super-initial" state which would make
     the output nondeterministic.  Normalization is only needed to make the
     determinized output more minimal anyway, it's not needed for correctness.
     Note, we don't put anything in the initial_hash_.  The initial_hash_ is only
     a lookaside buffer anyway, so this isn't a problem-- it will get populated
     later if it needs to be.
  */
        std::vector<Element> subset(1);
        subset[0].state = start_id;
        subset[0].weight = KdLatticeWeight::One();
        subset[0].string = repository_.EmptyString();  // Id of empty sequence.
        EpsilonClosure(&subset); // follow through epsilon-input links
        ConvertToMinimal(&subset); // remove all but final states and
        // states with input-labels on arcs out of them.
        // KdLatticeWeight::One() is the "forward-weight" of this determinized state...
        // i.e. the minimal cost from the start of the determinized FST to this
        // state [One() because it's the start state].
        OutputState *initial_state = new OutputState(subset, 0);
        KALDI_ASSERT(output_states_.empty());
        output_states_.push_back(initial_state);
        num_elems_ += subset.size();
        KdStateId initial_state_id = 0;
        minimal_hash_[&(initial_state->minimal_subset)] = initial_state_id;
        ProcessFinal(initial_state_id);
        ProcessTransitions(initial_state_id); // this will add tasks to
        // the queue, which we'll start processing in Determinize().
    }
}

void KdLatDet::AddStrings(const std::vector<Element> &vec,
                std::vector<StringId> *needed_strings)
{
    for (typename std::vector<Element>::const_iterator iter = vec.begin();
         iter!=vec.end(); ++iter)
        needed_strings->push_back(iter->string);
}
