#include "kd_lattice.h"

using namespace kaldi;

void kd_fstSSPathBacktrace(Lattice *ifst, Lattice *ofst,
    std::vector<std::pair<KdStateId, size_t>> &parent,KdStateId f_parent)
{
    ofst->DeleteStates();
    ofst->SetInputSymbols(ifst->InputSymbols());
    ofst->SetOutputSymbols(ifst->OutputSymbols());
    KdStateId s_p = KD_INVALID_STATE;
    KdStateId d_p = KD_INVALID_STATE;
    for( KdStateId state = f_parent, d = KD_INVALID_STATE; state!=KD_INVALID_STATE;
         d = state, state=parent[state].first )
    {
        d_p = s_p;
        s_p = ofst->AddState();
        if( d==KD_INVALID_STATE )
        {
            ofst->SetFinal(s_p, ifst->Final(f_parent));
        }
        else
        {
            fst::ArcIterator<fst::Fst<LatticeArc>> aiter(*ifst, state);
            aiter.Seek(parent[d].second);
            auto arc = aiter.Value();
            arc.nextstate = d_p;
            ofst->AddArc(s_p, std::move(arc));
        }
    }
    ofst->SetStart(s_p);
    if( ifst->Properties(FST_ERROR, false) )
    {
        ofst->SetProperties(FST_ERROR, FST_ERROR);
    }
    uint64 ofst_property = ofst->Properties(FST_PROPERTY, false);
    uint64 path_property = fst::ShortestPathProperties(ofst_property, true);
    ofst->SetProperties(path_property , FST_PROPERTY);
}

bool kd_SingleShortestPath(Lattice *ifst, KdStateId *f_parent,
                           std::vector<std::pair<KdStateId, size_t>> *parent)
{
    std::vector<LatticeArc::Weight> distance;
    parent->clear();
    fst::AnyArcFilter<LatticeArc> arc_filter;
    fst::AutoQueue<KdStateId> state_queue(*ifst, &distance, arc_filter);

    if( ifst->Start()==KD_INVALID_STATE )
    {
        qDebug() << "kd_SingleShortestPath: ifst->Start() is KD_INVALID_STATE";
        return true;
    }
    std::vector<bool> enqueued;
    KdStateId source = ifst->Start();
    bool final_seen = false;
    LatticeArc::Weight f_distance = LatticeArc::Weight::Zero();
    distance.clear();
    state_queue.Clear();

    while( distance.size()<source )
    {
        distance.push_back(LatticeArc::Weight::Zero());
        enqueued.push_back(false);
        parent->emplace_back(KD_INVALID_STATE, KD_INVALID_ARC);
    }

    distance.push_back(LatticeArc::Weight::One());
    parent->emplace_back(KD_INVALID_STATE, KD_INVALID_ARC);
    state_queue.Enqueue(source);
    enqueued.push_back(true);
    while (!state_queue.Empty())
    {
        const auto s = state_queue.Head();
        state_queue.Dequeue();
        enqueued[s] = false;
        const auto sd = distance[s];
        // If we are using a shortest queue, no other path is going to be shorter
        // than f_distance at this point.

        if (ifst->Final(s) != LatticeArc::Weight::Zero())
        {
            const auto plus = Plus(f_distance, Times(sd, ifst->Final(s)));
            if( f_distance!=plus )
            {
                f_distance = plus;
                *f_parent = s;
            }
            if( !f_distance.Member() )
                return false;
            final_seen = true;
        }

        for( fst::ArcIterator<fst::Fst<LatticeArc>> aiter(*ifst, s); !aiter.Done(); aiter.Next() )
        {
            const auto &arc = aiter.Value();
            while (distance.size() <= arc.nextstate)
            {
                distance.push_back(LatticeArc::Weight::Zero());
                enqueued.push_back(false);
                parent->emplace_back(KD_INVALID_STATE, KD_INVALID_ARC);
            }

            auto &nd = distance[arc.nextstate];
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
    std::vector<std::pair<KdStateId, size_t>> parent;
    KdStateId f_parent = KD_INVALID_STATE;

    if( kd_SingleShortestPath(ifst, &f_parent, &parent) )
    {
        kd_fstSSPathBacktrace(ifst, ofst, parent, f_parent);
    }
    else
    {
        qDebug() << "kd_ShortestPath is fucked";
    }
    return;
}
