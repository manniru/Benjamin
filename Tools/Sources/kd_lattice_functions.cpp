#include "kd_lattice.h"

using namespace kaldi;

void kd_latticeGetTimes(KdLattice *lat, std::vector<int> *times)
{
    if( !lat->Properties(fst::kTopSorted, true) )
    {
        KALDI_ERR << "Input lattice must be topologically sorted.";
    }

    KALDI_ASSERT(lat->Start() == 0);
    int num_states = lat->NumStates();

    times->clear();
    times->resize(num_states, -1);

    (*times)[0] = 0;
    for( int state=0 ; state<num_states; state++ )
    {
        int cur_time = (*times)[state];
        for (fst::ArcIterator<KdLattice> aiter(*lat, state); !aiter.Done();
             aiter.Next())
        {
            const LatticeArc &arc = aiter.Value();

            if( arc.ilabel!=0 )
            {
                if ((*times)[arc.nextstate] == -1)
                {
                    (*times)[arc.nextstate] = cur_time + 1;
                }
            }
            else //epsilon
            {
                // Same time instance
                if( (*times)[arc.nextstate]==-1 )
                {
                    (*times)[arc.nextstate] = cur_time;
                }
            }
        }
    }
}

bool kd_PruneLattice(float beam, KdLattice *lat)
{
    typedef typename KdLattice::Arc Arc;
    typedef typename Arc::Weight Weight;
    typedef typename Arc::StateId StateId;

    KALDI_ASSERT(beam > 0.0);
    if (!lat->Properties(fst::kTopSorted, true))
    {
        if (fst::TopSort(lat) == false)
        {
            KALDI_WARN << "Cycles detected in lattice";
            return false;
        }
    }
    // We assume states before "start" are not reachable, since
    // the lattice is topologically sorted.
    int32 start = lat->Start();
    int32 num_states = lat->NumStates();
    if (num_states == 0)
        return false;
    std::vector<double> forward_cost(num_states,
                                     std::numeric_limits<double>::infinity());  // viterbi forward.
    forward_cost[start] = 0.0; // lattice can't have cycles so couldn't be
    // less than this.
    double best_final_cost = std::numeric_limits<double>::infinity();
    // Update the forward probs.
    // Thanks to Jing Zheng for finding a bug here.
    for (int32 state = 0; state < num_states; state++)
    {
        double this_forward_cost = forward_cost[state];
        for (fst::ArcIterator<KdLattice> aiter(*lat, state);
             !aiter.Done(); aiter.Next())
        {
            const Arc &arc(aiter.Value());
            StateId nextstate = arc.nextstate;
            KALDI_ASSERT(nextstate > state && nextstate < num_states);
            double next_forward_cost = this_forward_cost +
                    ConvertToCost(arc.weight);
            if (forward_cost[nextstate] > next_forward_cost)
                forward_cost[nextstate] = next_forward_cost;
        }
        Weight final_weight = lat->Final(state);
        double this_final_cost = this_forward_cost +
                ConvertToCost(final_weight);
        if (this_final_cost < best_final_cost)
            best_final_cost = this_final_cost;
    }
    int32 bad_state = lat->AddState(); // this state is not final.
    double cutoff = best_final_cost + beam;

    // Go backwards updating the backward probs (which share memory with the
    // forward probs), and pruning arcs and deleting final-probs.  We prune arcs
    // by making them point to the non-final state "bad_state".  We'll then use
    // Trim() to remove unnecessary arcs and states.  [this is just easier than
    // doing it ourselves.]
    std::vector<double> &backward_cost(forward_cost);
    for (int32 state = num_states - 1; state >= 0; state--) {
        double this_forward_cost = forward_cost[state];
        double this_backward_cost = ConvertToCost(lat->Final(state));
        if (this_backward_cost + this_forward_cost > cutoff
                && this_backward_cost != std::numeric_limits<double>::infinity())
            lat->SetFinal(state, Weight::Zero());
        for (fst::MutableArcIterator<KdLattice> aiter(lat, state);
             !aiter.Done();
             aiter.Next())
        {
            Arc arc(aiter.Value());
            StateId nextstate = arc.nextstate;
            KALDI_ASSERT(nextstate > state && nextstate < num_states);
            double arc_cost = ConvertToCost(arc.weight),
                    arc_backward_cost = arc_cost + backward_cost[nextstate],
                    this_fb_cost = this_forward_cost + arc_backward_cost;
            if (arc_backward_cost < this_backward_cost)
                this_backward_cost = arc_backward_cost;
            if (this_fb_cost > cutoff) { // Prune the arc.
                arc.nextstate = bad_state;
                aiter.SetValue(arc);
            }
        }
        backward_cost[state] = this_backward_cost;
    }
    fst::Connect(lat);
    return (lat->NumStates() > 0);
}
