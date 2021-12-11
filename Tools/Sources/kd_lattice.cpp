#include "kd_lattice.h"

using namespace kaldi;

bool kd_SingleShortestPath(Lattice *ifst, std::vector<LatticeArc::Weight> *distance,
        KdStateId *f_parent, std::vector<std::pair<KdStateId, size_t>> *parent)
{
    parent->clear();
    fst::AnyArcFilter<LatticeArc> arc_filter;
    fst::AutoQueue<KdStateId> state_queue(*ifst, distance, arc_filter);
    *f_parent = KD_INVALID_STATE;

    if( ifst->Start()==KD_INVALID_STATE )
        return true;
    std::vector<bool> enqueued;
    KdStateId source = ifst->Start();
    bool final_seen = false;
    LatticeArc::Weight f_distance = LatticeArc::Weight::Zero();
    distance->clear();
    state_queue.Clear();
    while (distance->size() < source)
    {
        distance->push_back(LatticeArc::Weight::Zero());
        enqueued.push_back(false);
        parent->emplace_back(KD_INVALID_STATE, KD_INVALID_ARC);
    }
    distance->push_back(LatticeArc::Weight::One());
    parent->emplace_back(KD_INVALID_STATE, KD_INVALID_ARC);
    state_queue.Enqueue(source);
    enqueued.push_back(true);
    while (!state_queue.Empty())
    {
        const auto s = state_queue.Head();
        state_queue.Dequeue();
        enqueued[s] = false;
        const auto sd = (*distance)[s];
        // If we are using a shortest queue, no other path is going to be shorter
        // than f_distance at this point.

        if (ifst->Final(s) != LatticeArc::Weight::Zero())
        {
            const auto plus = Plus(f_distance, Times(sd, ifst->Final(s)));
            if (f_distance != plus)
            {
                f_distance = plus;
                *f_parent = s;
            }
            if (!f_distance.Member())
                return false;
            final_seen = true;
        }

        for( fst::ArcIterator<fst::Fst<LatticeArc>> aiter(*ifst, s); !aiter.Done(); aiter.Next() )
        {
            const auto &arc = aiter.Value();
            while (distance->size() <= arc.nextstate)
            {
                distance->push_back(LatticeArc::Weight::Zero());
                enqueued.push_back(false);
                parent->emplace_back(KD_INVALID_STATE, KD_INVALID_ARC);
            }

            auto &nd = (*distance)[arc.nextstate];
            const auto weight = Times(sd, arc.weight);

            if( nd!=Plus(nd, weight) )
            {
                nd = Plus(nd, weight);
                if (!nd.Member())
                    return false;
                (*parent)[arc.nextstate] = std::make_pair(s, aiter.Position());
                if (!enqueued[arc.nextstate])
                {
                    state_queue.Enqueue(arc.nextstate);
                    enqueued[arc.nextstate] = true;
                }
                else
                {
                    state_queue.Update(arc.nextstate);
                }
            }
        }
    }
    return true;
}

void kd_fstShortestPath(Lattice *ifst, Lattice *ofst)
{
    std::vector<LatticeArc::Weight> distance;
    std::vector<std::pair<KdStateId, size_t>> parent;
    KdStateId f_parent;

    if( kd_SingleShortestPath(ifst, &distance, &f_parent, &parent))
    {
        fst::internal::SingleShortestPathBacktrace(*ifst, ofst, parent, f_parent);
    }
    else
    {
        qDebug() << "kd_ShortestPath is fucked";
    }
    return;
}
