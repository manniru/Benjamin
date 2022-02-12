#ifndef KD_LATTICE_DET_H
#define KD_LATTICE_DET_H

#include<QVector>
#include "kd_token2.h"
#include "kd_lattice.h"
#include <fstext/determinize-lattice-inl.h>

// A representable float near .001.
#define KD_KDELTA fst::kDelta

//KdDetOpt
struct KdDetOpt
{
    float delta = KD_KDELTA; // A small offset used to measure equality of weights.
    int max_mem = -1; // If >0, determinization will fail and return false
    // when the algorithm's (approximate) memory consumption crosses this threshold.
    int max_loop = -1; // If >0, can be used to detect non-determinizable input
    // (a case that wouldn't be caught by max_mem).
    int max_states = -1 ;
    int max_arcs = -1;
    float retry_cutoff = 0.5;
};

// [note: the output symbols would usually be
// p.d.f. id's in the anticipated use of this code] It has a special requirement
// on the KdLatticeWeight type: that there should be a Compare function on the weights
// such that Compare(w1, w2) returns -1 if w1 < w2, 0 if w1 == w2, and +1 if w1 >
// w2.  This requires that there be a total order on the weights.
// LatticeDeterminizerPruned
class KdLatDet
{
public:
    // Output to Gallic acceptor (so the strings go on weights, and there is a 1-1 correspondence
    // between our states and the states in ofst.  If destroy == true, release memory as we go
    // (but we cannot output again).

    // Output to standard FST with CompactWeightTpl<KdLatticeWeight> as its weight type (the
    // weight stores the original output-symbol strings).  If destroy == true,
    // release memory as we go (but we cannot output again).
    void Output(kaldi::CompactLattice  *ofst, bool destroy = true);

    // Output to standard FST with KdLatticeWeight as its weight type.  We will create extra
    // states to handle sequences of symbols on the output.  If destroy == true,
    // release memory as we go (but we cannot output again).
    void  Output(KdLattice *ofst, bool destroy = true) ;


    // Initializer.  After initializing the object you will typically
    // call Determinize() and then call one of the Output functions.
    // Note: ifst.Copy() will generally do a
    // shallow copy.  We do it like this for memory safety, rather than
    // keeping a reference or pointer to ifst_.
    KdLatDet(KdLattice &ifst, double beam, KdDetOpt opts);
    ~KdLatDet();

    void FreeOutputStates();

    // frees all memory except the info (in output_states_[ ]->arcs)
    // that we need to output the FST.
    void FreeMostMemory();

    void RebuildRepository();

    bool CheckMemoryUsage();

    bool Determinize(double *effective_beam);
private:

    typedef typename KdLatticeArc::Label Label;
    typedef typename KdLatticeArc::StateId StateId;  // use this when we don't know if it's input or output.
    typedef typename KdLatticeArc::StateId InputStateId;  // state in the input FST.
    typedef typename KdLatticeArc::StateId KdStateId;  // same as above but distinguish
    // states in output Fst.

    typedef fst::LatticeStringRepository<int> StringRepositoryType;
    typedef const typename StringRepositoryType::Entry* StringId;

    // Element of a subset [of original states]
    struct Element
    {
        StateId state; // use StateId as this is usually InputStateId but in one case
        // KdStateId.
        StringId string;
        KdLatticeWeight weight;
        bool operator != (const Element &other) const {
            return (state != other.state || string != other.string ||
                    weight != other.weight);
        }
        // This operator is only intended for the priority_queue in the function
        // EpsilonClosure().
        bool operator > (const Element &other) const {
            return state > other.state;
        }
        // This operator is only intended to support sorting in EpsilonClosure()
        bool operator < (const Element &other) const {
            return state < other.state;
        }
    };

    // Arcs in the format we temporarily create in this class (a representation, essentially of
    // a Gallic Fst).
    struct TempArc {
        Label ilabel;
        StringId string;  // Look it up in the StringRepository, it's a sequence of Labels.
        KdStateId nextstate;  // or kNoState for final weights.
        KdLatticeWeight weight;
    };

    // Hashing function used in hash of subsets.
    // A subset is a pointer to std::vector<Element>.
    // The Elements are in sorted order on state id, and without repeated states.
    // Because the order of Elements is fixed, we can use a hashing function that is
    // order-dependent.  However the weights are not included in the hashing function--
    // we hash subsets that differ only in weight to the same key.  This is not optimal
    // in terms of the O(N) performance but typically if we have a lot of determinized
    // states that differ only in weight then the input probably was pathological in some way,
    // or even non-determinizable.
    //   We don't quantize the weights, in order to avoid inexactness in simple cases.
    // Instead we apply the delta when comparing subsets for equality, and allow a small
    // difference.

    class SubsetKey {
    public:
        size_t operator ()(const std::vector<Element> * subset) const {  // hashes only the state and string.
            size_t hash = 0, factor = 1;
            for (typename std::vector<Element>::const_iterator iter= subset->begin(); iter != subset->end(); ++iter) {
                hash *= factor;
                hash += iter->state + reinterpret_cast<size_t>(iter->string);
                factor *= 23531;  // these numbers are primes.
            }
            return hash;
        }
    };

    // This is the equality operator on subsets.  It checks for exact match on state-id
    // and string, and approximate match on weights.
    class SubsetEqual {
    public:
        bool operator ()(const std::vector<Element> * s1, const std::vector<Element> * s2) const {
            size_t sz = s1->size();
            KALDI_ASSERT(sz>=0);
            if (sz != s2->size()) return false;
            typename std::vector<Element>::const_iterator iter1 = s1->begin(),
                    iter1_end = s1->end(), iter2=s2->begin();
            for (; iter1 < iter1_end; ++iter1, ++iter2) {
                if (iter1->state != iter2->state ||
                        iter1->string != iter2->string ||
                        ! ApproxEqual(iter1->weight, iter2->weight, delta_)) return false;
            }
            return true;
        }
        float delta_;
        SubsetEqual(float delta): delta_(delta) {}
        SubsetEqual(): delta_(KD_KDELTA) {}
    };

    // Operator that says whether two Elements have the same states.
    // Used only for debug.
    class SubsetEqualStates {
    public:
        bool operator ()(const std::vector<Element> * s1, const std::vector<Element> * s2) const {
            size_t sz = s1->size();
            KALDI_ASSERT(sz>=0);
            if (sz != s2->size()) return false;
            typename std::vector<Element>::const_iterator iter1 = s1->begin(),
                    iter1_end = s1->end(), iter2=s2->begin();
            for (; iter1 < iter1_end; ++iter1, ++iter2) {
                if (iter1->state != iter2->state) return false;
            }
            return true;
        }
    };

    // Define the hash type we use to map subsets (in minimal
    // representation) to KdStateId.
    typedef unordered_map<const std::vector<Element>*, KdStateId,
    SubsetKey, SubsetEqual> MinimalSubsetHash;

    // Define the hash type we use to map subsets (in initial
    // representation) to KdStateId, together with an
    // extra weight. [note: we interpret the Element.state in here
    // as an KdStateId even though it's declared as InputStateId;
    // these types are the same anyway].
    typedef unordered_map<const std::vector<Element>*, Element,
    SubsetKey, SubsetEqual> InitialSubsetHash;


    // converts the representation of the subset from canonical (all states) to
    // minimal (only states with output symbols on arcs leaving them, and final
    // states).  Output is not necessarily normalized, even if input_subset was.
    void ConvertToMinimal(std::vector<Element> *subset);

    // Takes a minimal, normalized subset, and converts it to an KdStateId.
    // Involves a hash lookup, and possibly adding a new KdStateId.
    // If it creates a new KdStateId, it creates a new record for it, works
    // out its final-weight, and puts stuff on the queue relating to its
    // transitions.
    KdStateId MinimalToStateId(const std::vector<Element> &subset,
                                   const double forward_cost);


    // Given a normalized initial subset of elements (i.e. before epsilon closure),
    // compute the corresponding output-state.
    KdStateId InitialToStateId(const std::vector<Element> &subset_in,
                                   double forward_cost,
                                   KdLatticeWeight *remaining_weight,
                                   StringId *common_prefix);

    // returns the Compare value (-1 if a < b, 0 if a == b, 1 if a > b) according
    // to the ordering we defined on strings for the CompactLatticeWeightTpl.
    // see function
    // inline int Compare (const CompactLatticeWeightTpl<WeightType,IntType> &w1,
    //                     const CompactLatticeWeightTpl<WeightType,IntType> &w2)
    // in lattice-weight.h.
    // this is the same as that, but optimized for our data structures.
    int Compare(const KdLatticeWeight &a_w, StringId a_str,
                       const KdLatticeWeight &b_w, StringId b_str);

    // This function computes epsilon closure of subset of states by following epsilon links.
    // Called by InitialToStateId and Initialize.
    // Has no side effects except on the string repository.  The "output_subset" is not
    // necessarily normalized (in the sense of there being no common substring), unless
    // input_subset was.
    void EpsilonClosure(std::vector<Element> *subset);

    // This function works out the final-weight of the determinized state.
    // called by ProcessSubset.
    // Has no side effects except on the variable repository_, and
    // output_states_[output_state_id].arcs
    void ProcessFinal(KdStateId output_state_id);

    // NormalizeSubset normalizes the subset "elems" by
    // removing any common string prefix (putting it in common_str),
    // and dividing by the total weight (putting it in tot_weight).
    void NormalizeSubset(std::vector<Element> *elems,
                         KdLatticeWeight *tot_weight,
                         StringId *common_str);

    // Take a subset of Elements that is sorted on state, and
    // merge any Elements that have the same state (taking the best
    // (weight, string) pair in the semiring).
    void MakeSubsetUnique(std::vector<Element> *subset);

    // ProcessTransition was called from "ProcessTransitions" in the non-pruned
    // code, but now we in effect put the calls to ProcessTransition on a priority
    // queue, and it now gets called directly from Determinize().  This function
    // processes a transition from state "ostate_id".  The set "subset" of Elements
    // represents a set of next-states with associated weights and strings, each
    // one arising from an arc from some state in a determinized-state; the
    // next-states are unique (there is only one Entry assocated with each)
    void ProcessTransition(KdStateId ostate_id, Label ilabel, std::vector<Element> *subset);


    // "less than" operator for pair<Label, Element>.   Used in ProcessTransitions.
    // Lexicographical order, which only compares the state when ordering the
    // "Element" member of the pair.

    class PairComparator {
    public:
        inline bool operator () (const std::pair<Label, Element> &p1, const std::pair<Label, Element> &p2) {
            if (p1.first < p2.first) return true;
            else if (p1.first > p2.first) return false;
            else {
                return p1.second.state < p2.second.state;
            }
        }
    };

    void ProcessTransitions(KdStateId output_state_id);
    bool IsIsymbolOrFinal(InputStateId state);

    void ComputeBackwardWeight();
    void InitializeDeterminization();

    struct OutputState {
        std::vector<Element> minimal_subset;
        std::vector<TempArc> arcs; // arcs out of the state-- those that have been processed.
        // Note: the final-weight is included here with kNoStateId as the state id.  We
        // always process the final-weight regardless of the beam; when producing the
        // output we may have to ignore some of these.
        double forward_cost; // Represents minimal cost from start-state
        // to this state.  Used in prioritization of tasks, and pruning.
        // Note: we know this minimal cost from when we first create the OutputState;
        // this is because of the priority-queue we use, that ensures that the
        // "best" path into the state will be expanded first.
        OutputState(const std::vector<Element> &minimal_subset,
                    double forward_cost): minimal_subset(minimal_subset),
            forward_cost(forward_cost) { }
    };

    std::vector<OutputState*> output_states_; // All the info about the output states.

    int num_arcs_; // keep track of memory usage: number of arcs in output_states_[ ]->arcs
    int num_elems_; // keep track of memory usage: number of elems in output_states_ and
    // the keys of initial_hash_

    KdLattice *ifst_;
    std::vector<double> backward_costs_; // This vector stores, for every state in ifst_,
    // the minimal cost to the end-state (i.e. the sum of weights; they are guaranteed to
    // have "take-the-minimum" semantics).  We get the double from the ConvertToCost()
    // function on the lattice weights.

    double beam_;
    double cutoff_; // beam plus total-weight of input (and note, the weight is
    // guaranteed to be "tropical-like" so the sum does represent a min-cost.

    KdDetOpt opts_;
    SubsetKey hasher_;  // object that computes keys-- has no data members.
    SubsetEqual equal_;  // object that compares subsets-- only data member is delta_.
    bool determinized_; // set to true when user called Determinize(); used to make
    // sure this object is used correctly.
    MinimalSubsetHash minimal_hash_;  // hash from Subset to KdStateId.  Subset is "minimal
    // representation" (only include final and states and states with
    // nonzero ilabel on arc out of them.  Owns the pointers
    // in its keys.
    InitialSubsetHash initial_hash_;   // hash from Subset to Element, which
    // represents the KdStateId together
    // with an extra weight and string.  Subset
    // is "initial representation".  The extra
    // weight and string is needed because after
    // we convert to minimal representation and
    // normalize, there may be an extra weight
    // and string.  Owns the pointers
    // in its keys.

    struct Task {
        KdStateId state; // State from which we're processing the transition.
        Label label; // Label on the transition we're processing out of this state.
        std::vector<Element> subset; // Weighted subset of states (with strings)-- not normalized.
        double priority_cost; // Cost used in deciding priority of tasks.  Note:
        // we assume there is a ConvertToCost() function that converts the semiring to double.
    };

    struct TaskCompare
    {
        inline int operator() (const Task *t1, const Task *t2) {
            // view this like operator <, which is the default template parameter
            // to std::priority_queue.
            // returns true if t1 is worse than t2.
            return (t1->priority_cost > t2->priority_cost);
        }
    };

    // This priority queue contains "Task"s to be processed; these correspond
    // to transitions out of determinized states.  We process these in priority
    // order according to the best weight of any path passing through these
    // determinized states... it's possible to work this out.
    std::priority_queue<Task*, std::vector<Task*>, TaskCompare> queue_;

    std::vector<std::pair<Label, Element> > all_elems_tmp_; // temporary vector used in ProcessTransitions.

    enum IsymbolOrFinal { OSF_UNKNOWN = 0, OSF_NO = 1, OSF_YES = 2 };

    std::vector<char> isymbol_or_final_; // A kind of cache; it says whether
    // each state is (emitting or final) where emitting means it has at least one
    // non-epsilon output arc.  Only accessed by IsIsymbolOrFinal()

    fst::LatticeStringRepository<int> repository_;  // defines a compact and fast way of
    // storing sequences of labels.

    void AddStrings(const std::vector<Element> &vec,
                    std::vector<StringId> *needed_strings);
};


#endif // KD_LATTICE_DET_H
