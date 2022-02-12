#include "kd_lattice_functions.h"

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

// DeterminizeLatticePhonePrunedWrapper
bool kd_detLatPhonePrunedW(TransitionModel &trans_model,
                           KdLattice *ifst,
                           double beam, CompactLattice *ofst,
                           KdPrunedOpt opts)
{
    bool ans = true;
    Invert(ifst);
    if (ifst->Properties(fst::kTopSorted, true) == 0)
    {
        if (!TopSort(ifst))
        {
            KALDI_ERR << "Topological sorting lattice failed.";
        }
    }
    fst::ILabelCompare<KdLatticeArc> ilabel_comp;
    fst::ArcSort(ifst, ilabel_comp);
    ans = kd_detLatPhonePruned(trans_model, ifst,
                               beam, ofst, opts);
    Connect(ofst);
    return ans;
}

// DeterminizeLatticePhonePruned
bool kd_detLatPhonePruned(kaldi::TransitionModel &trans_model,
                          KdLattice *ifst, double beam,
                          CompactLattice *ofst, KdPrunedOpt opts)
{
    // Returning status.
    bool ans = true;

    // Make sure at least one of opts.phone_determinize and opts.word_determinize
    // is not false, otherwise calling this function doesn't make any sense.
    if ((opts.phone_determinize || opts.word_determinize) == false)
    {
        KALDI_WARN << "Both --phone-determinize and --word-determinize are set to "
                   << "false, copying lattice without determinization.";
        // We are expecting the words on the input side.
        fst::ConvertLattice<KdLatticeWeight, int>(*ifst, ofst, false);
        return ans;
    }

    // Determinization options.
    KdDetOpt det_opts;
    det_opts.delta = opts.delta;
    det_opts.max_mem = opts.max_mem;

    // If --phone-determinize is true, do the determinization on phone + word
    // lattices.
    if (opts.phone_determinize)
    {
        KALDI_VLOG(3) << "Doing first pass of determinization on phone + word "
                      << "lattices.";
        ans = kd_DetLatFirstPass(trans_model, beam, ifst, &det_opts) && ans;

        // If --word-determinize is false, we've finished the job and return here.
        if (!opts.word_determinize)
        {
            // We are expecting the words on the input side.
            fst::ConvertLattice<KdLatticeWeight, int>(*ifst, ofst, false);
            return ans;
        }
    }

    // If --word-determinize is true, do the determinization on word lattices.
    if (opts.word_determinize)
    {
        KALDI_VLOG(3) << "Doing second pass of determinization on word lattices.";
        ans = kd_detLatPruned(*ifst, beam,
                              ofst, &det_opts) && ans;
    }

    return ans;
}

//DeterminizeLatticePhonePrunedFirstPass
bool kd_DetLatFirstPass(kaldi::TransitionModel &trans_model,
                        double beam, KdLattice *fst, KdDetOpt *opts)
{
    // First, insert the phones.
    KdLatticeArc::Label first_phone_label =
            DeterminizeLatticeInsertPhones(trans_model, fst);
    TopSort(fst);

    // Second, do determinization with phone inserted.
    bool ans = kd_detLatPruned(*fst, beam, fst, *opts);

    // Finally, remove the inserted phones.
    fst::DeterminizeLatticeDeletePhones(first_phone_label, fst);
    TopSort(fst);

    return ans;
}


// DeterminizeLatticePruned
// there are two versions with different output FST types.
bool kd_detLatPruned(KdLattice &ifst, double beam,
                     KdLattice *ofst, KdDetOpt opts)
{
    ofst->SetInputSymbols(ifst.InputSymbols());
    ofst->SetOutputSymbols(ifst.OutputSymbols());
    if (ifst.NumStates() == 0)
    {
        ofst->DeleteStates();
        return true;
    }
    KALDI_ASSERT(opts.retry_cutoff >= 0.0 && opts.retry_cutoff < 1.0);
    int32 max_num_iters = 10;  // avoid the potential for infinite loops if
    // retrying.
    KdLattice temp_fst;

    for (int32 iter = 0; iter < max_num_iters; iter++)
    {
        KdLatDet det(iter == 0 ? ifst : temp_fst, beam, opts);
        double effective_beam;
        bool ans = det.Determinize(&effective_beam);
        // if it returns false it will typically still produce reasonable output,
        // just with a narrower beam than "beam".  If the user specifies an infinite
        // beam we don't do this beam-narrowing.
        if (effective_beam >= beam * opts.retry_cutoff ||
                beam == std::numeric_limits<double>::infinity() ||
                iter + 1 == max_num_iters)
        {
            det.Output(ofst);
            return ans;
        }
        else
        {
            // The code below to set "beam" is a heuristic.
            // If effective_beam is very small, we want to reduce by a lot.
            // But never change the beam by more than a factor of two.
            if (effective_beam < 0.0) effective_beam = 0.0;
            double new_beam = beam * sqrt(effective_beam / beam);
            if (new_beam < 0.5 * beam) new_beam = 0.5 * beam;
            beam = new_beam;
            if (iter == 0)
                temp_fst = ifst;
            kd_PruneLattice(beam, &temp_fst);
            KALDI_LOG << "Pruned state-level lattice with beam " << beam
                      << " and retrying determinization with that beam.";
        }
    }
    return false; // Suppress compiler warning; this code is unreachable.
}

bool kd_detLatPruned( KdLattice &ifst, double beam,
                      CompactLattice *ofst, KdDetOpt opts)
{
    ofst->SetInputSymbols(ifst.InputSymbols());
    ofst->SetOutputSymbols(ifst.OutputSymbols());
    if (ifst.NumStates() == 0)
    {
        ofst->DeleteStates();
        return true;
    }
    KALDI_ASSERT(opts.retry_cutoff >= 0.0 && opts.retry_cutoff < 1.0);
    int max_num_iters = 10;  // avoid the potential for infinite loops if
    // retrying.
    KdLattice temp_fst;

    for (int32 iter = 0; iter < max_num_iters; iter++)
    {
        LatticeDeterminizerPruned<Weight, IntType> det(iter == 0 ? ifst : temp_fst,
                                                       beam, opts);
        double effective_beam;
        bool ans = det.Determinize(&effective_beam);
        // if it returns false it will typically still produce reasonable output,
        // just with a narrower beam than "beam".  If the user specifies an infinite
        // beam we don't do this beam-narrowing.
        if (effective_beam >= beam * opts.retry_cutoff ||
                beam == std::numeric_limits<double>::infinity() ||
                iter + 1 == max_num_iters) {
            det.Output(ofst);
            return ans;
        }
        else
        {
            // The code below to set "beam" is a heuristic.
            // If effective_beam is very small, we want to reduce by a lot.
            // But never change the beam by more than a factor of two.
            if (effective_beam < 0.0) effective_beam = 0.0;
            double new_beam = beam * sqrt(effective_beam / beam);
            if (new_beam < 0.5 * beam) new_beam = 0.5 * beam;
            beam = new_beam;
            if (iter == 0) temp_fst = ifst;
            kd_PruneLattice(beam, &temp_fst);
            KALDI_LOG << "Pruned state-level lattice with beam " << beam
                      << " and retrying determinization with that beam.";
        }
    }
    return false; // Suppress compiler warning; this code is unreachable.
}
