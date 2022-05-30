#include "kd_lattice_prune.h"

KdPrune::KdPrune(float beam)
{
//    KALDI_ASSERT(beam > 0.0);
    lattice_beam = beam;
}

bool KdPrune::prune(KdLattice *lat)
{
    if( !lat->Properties(fst::kTopSorted, true) )
    {
        qDebug() << "THIS IS NOT POSSIBLE";
        exit(0);
    }

    num_states = lat->NumStates();
    if( num_states==0 )
    {
        return false;
    }
    double cutoff = getCutOff(lat);

    // Go backwards updating the backward probs, and pruning arcs and deleting final-probs.
    // We prune arcs by making them point to "bad_state".
    int bad_state = lat->AddState(); // this state is not final.
    QVector<double> backward_cost = forward_cost;

    for (int32 state=num_states-1 ; state>=0 ; state-- )
    {
        double this_forward_cost = forward_cost[state];
        KdLatticeWeight w = lat->Final(state);
        double this_backward_cost = w.g_cost + w.a_cost;
        if( this_backward_cost + this_forward_cost > cutoff
                && this_backward_cost!=KD_INFINITY_DB)
        {
            lat->SetFinal(state, KdLatticeWeight::Zero());
        }

        for (fst::MutableArcIterator<KdLattice> aiter(lat, state);
             !aiter.Done(); aiter.Next())
        {
            KdLattice::Arc arc(aiter.Value());
            KdStateId nextstate = arc.nextstate;

            double arc_cost = arc.weight.getCost();
            double arc_backward_cost = arc_cost + backward_cost[nextstate];
            double this_fb_cost = this_forward_cost + arc_backward_cost;
            if( arc_backward_cost<this_backward_cost )
            {
                this_backward_cost = arc_backward_cost;
            }
            if( this_fb_cost>cutoff )
            {
                // Prune the arc.
                arc.nextstate = bad_state;
                aiter.SetValue(arc);
            }
        }
        backward_cost[state] = this_backward_cost;
    }
    // connect would remove all arcs bad states
    fst::Connect(lat);
    return( lat->NumStates()>0 );
}

double KdPrune::getCutOff(KdLattice *lat)
{
    int32 start = lat->Start();
    forward_cost.resize(num_states);
    for( int i=0 ; i<num_states ; i++ )
    {
        forward_cost[i] = KD_INFINITY_DB;
    }
    forward_cost[start] = 0.0; // lattice can't have cycles so couldn't be
    // less than this.

    double best_final_cost = KD_INFINITY_DB;
    // Update the forward probs.
    for( int32 state=0; state<num_states ; state++ )
    {
        double this_forward_cost = forward_cost[state];
        for (fst::ArcIterator<KdLattice> aiter(*lat, state);
             !aiter.Done(); aiter.Next())
        {
            KdLattice::Arc arc = aiter.Value();
            KdStateId nextstate = arc.nextstate;
//       KALDI_ASSERT(nextstate > state && nextstate < num_states);
            double next_forward_cost = this_forward_cost +
                    arc.weight.getCost();
            if( forward_cost[nextstate] > next_forward_cost)
                forward_cost[nextstate] = next_forward_cost;
        }
        KdLatticeWeight final_weight = lat->Final(state);
        double this_final_cost = this_forward_cost +
                final_weight.getCost();
        if( this_final_cost < best_final_cost)
            best_final_cost = this_final_cost;
    }
    double cutoff = best_final_cost + lattice_beam;
    return cutoff;
}
