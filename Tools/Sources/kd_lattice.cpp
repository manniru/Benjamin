#include "kd_lattice.h"

using namespace kaldi;

void kd_fstShortestPath(Lattice *ifst, Lattice *ofst)
{
    int nshortest = 1;
    bool unique = false;
    bool first_path = false;
    LatticeArc::Weight weight_threshold = LatticeArc::Weight::Zero();
    KdStateId state_threshold = KD_INVALID_STATE;
    float delta = KD_SHORTEST_DELTA;
    std::vector<LatticeArc::Weight> distance;
    fst::AnyArcFilter<LatticeArc> arc_filter;
    fst::AutoQueue<KdStateId> state_queue(*ifst, &distance, arc_filter);
    fst::ShortestPathOptions<LatticeArc, fst::AutoQueue<KdStateId>, fst::AnyArcFilter<LatticeArc>> opts(
                &state_queue, arc_filter, nshortest, unique, false, delta, first_path,
                weight_threshold, state_threshold);
    fst::ShortestPath(*ifst, ofst, &distance, opts);
}

// FinalRelativeCost()!=KD_INFINITY --> ReachedFinal

