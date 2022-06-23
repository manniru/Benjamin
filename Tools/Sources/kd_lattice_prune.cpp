#include "kd_lattice_prune.h"

KdPrune::KdPrune(float beam)
{
//    KALDI_ASSERT(beam > 0.0);
    lattice_beam = beam;
}

QString KdPrune::prune(KdLattice *lat)
{
    if( !lat->Properties(fst::kTopSorted, true) )
    {
        qDebug() << "prune need TopSort";
        if( !fst::TopSort(lat) )
        {
            qDebug() << "Topological sorting of lattice failed.";
            exit(0);
        }
    }

    num_states = lat->NumStates();
    if( num_states==0 )
    {
        return "";
    }
    double cutoff = getCutOff(lat);

    // Go backwards updating the backward probs, and pruning arcs and deleting final-probs.
    // We prune arcs by making them point to "bad_state".
    int bad_state = lat->AddState(); // this state is not final.
    QVector<double> backward_cost = forward_cost;
    QString dbg_times = " P0:";
    dbg_times += getLDiffTime();

    for( int state=num_states-1 ; state>=0 ; state-- )
    {
        KdLatticeWeight w = lat->Final(state);
        backward_cost[state] = w.getCost();
        double fb_cost = backward_cost[state] + forward_cost[state];
        if( fb_cost>cutoff && backward_cost[state]!=KD_INFINITY_DB )
        {//remove whole state
            lat->SetFinal(state, KdLatticeWeight::Zero());
        }

        for( fst::MutableArcIterator<KdLattice> aiter(lat, state) ;
             !aiter.Done() ; aiter.Next() )
        {
            KdLattice::Arc arc(aiter.Value());
            KdStateId nextstate = arc.nextstate;

            double arc_cost = arc.weight.getCost();
            double arc_backward_cost = arc_cost + backward_cost[nextstate];
            double this_fab_cost = forward_cost[state] + arc_backward_cost;
            if( backward_cost[state]>arc_backward_cost )
            {
                backward_cost[state] = arc_backward_cost;
            }
            if( this_fab_cost>cutoff )
            {
                // Prune the arc.
                arc.nextstate = bad_state;
                aiter.SetValue(arc);
            }
        }
    }

    fst::Connect(lat);
//    qDebug() << lat->NumStates()>0;
    return dbg_times;
}

double KdPrune::getCutOff(KdLattice *lat)
{
    int start = lat->Start();
    forward_cost.resize(num_states);
    for( int i=0 ; i<num_states ; i++ )
    {
        forward_cost[i] = KD_INFINITY_DB;
    }
    forward_cost[start] = 0.0;

    double best_final_cost = KD_INFINITY_DB;
    // Update the forward probs.
    for( int state=0; state<num_states ; state++ )
    {
        for( fst::ArcIterator<KdLattice> aiter(*lat, state);
             !aiter.Done(); aiter.Next())
        {
            KdLattice::Arc arc = aiter.Value();
            KdStateId nextstate = arc.nextstate;
            double next_forward_cost = forward_cost[state] +
                                       arc.weight.getCost();
            if( forward_cost[nextstate]>next_forward_cost )
            { //only if cost is lower update it
                forward_cost[nextstate] = next_forward_cost;
            }
        }

        KdLatticeWeight final_weight = lat->Final(state);
        if( final_weight.getCost()!=KD_INFINITY_DB )
        {
//            qDebug() << "state" << state << final_weight.getCost();
        }
        double final_cost = forward_cost[state] + final_weight.getCost();
        if( final_cost<best_final_cost )
        {
            best_final_cost = final_cost;
        }
    }
//    if( best_final_cost!=KD_INFINITY_DB )
//    {
//        exit(0);
//    }
    double cutoff = best_final_cost + lattice_beam;
    return cutoff;
}
