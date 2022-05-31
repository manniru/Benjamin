#include "kd_lattice.h"
#include "kd_lattice_compact.h"
#include "util/kaldi-io.h"
#include <QDebug>

void kd_fstSSPathBacktrace(KdLattice *ifst, KdLattice *ofst,
    std::vector<std::pair<KdStateId, size_t>> &parent,KdStateId f_parent)
{
    ofst->DeleteStates();
    ofst->SetInputSymbols(ifst->InputSymbols());
    ofst->SetOutputSymbols(ifst->OutputSymbols());
    KdStateId s_p = KD_INVALID_STATE;
    KdStateId d_p = KD_INVALID_STATE;
    KdStateId d   = KD_INVALID_STATE;

    for( KdStateId state = f_parent ; state!=KD_INVALID_STATE ; state=parent[state].first )
    {
        d_p = s_p;
        s_p = ofst->AddState();
        if( d==KD_INVALID_STATE )
        {
            ofst->SetFinal(s_p, ifst->Final(f_parent));
        }
        else
        {
            fst::ArcIterator<fst::Fst<KdLatticeArc>> aiter(*ifst, state);
            aiter.Seek(parent[d].second);
            auto arc = aiter.Value();
            arc.nextstate = d_p;
            ofst->AddArc(s_p, std::move(arc));
        }
        d = state;
    }
    ofst->SetStart(s_p);
    if( ifst->Properties(FST_ERROR, false) )
    {
        qDebug() << "kd_fstSSPathBacktrace FST_ERROR";
        ofst->SetProperties(FST_ERROR, FST_ERROR);
    }
    uint64 ofst_property = ofst->Properties(FST_PROPERTY, false);
    uint64 path_property = fst::ShortestPathProperties(ofst_property, true);
    ofst->SetProperties(path_property , FST_PROPERTY);
}

bool kd_SingleShortestPath(KdLattice *ifst, KdStateId *f_parent,
                           std::vector<std::pair<KdStateId, size_t>> *parent)
{
    std::vector<KdLatticeWeight> distance;
    parent->clear();
    fst::AnyArcFilter<KdLatticeArc> arc_filter;
    fst::AutoQueue<KdStateId> state_queue(*ifst, &distance, arc_filter);

    if( ifst->Start()==KD_INVALID_STATE )
    {
        qDebug() << "kd_SingleShortestPath: ifst->Start() is KD_INVALID_STATE";
        return true;
    }
    std::vector<bool> enqueued;
    KdStateId source = ifst->Start();
    KdLatticeWeight f_distance = KdLatticeWeight::Zero();
    distance.clear();
    state_queue.Clear();

    while( distance.size()<source )
    {
        distance.push_back(KdLatticeWeight::Zero());
        enqueued.push_back(false);
        parent->emplace_back(KD_INVALID_STATE, KD_INVALID_ARC);
    }

    distance.push_back(KdLatticeWeight::One());
    parent->emplace_back(KD_INVALID_STATE, KD_INVALID_ARC);
    state_queue.Enqueue(source);
    enqueued.push_back(true);
    while( !state_queue.Empty() )
    {
        const KdStateId s = state_queue.Head();
//        qDebug() << "state_queue" << s;
        state_queue.Dequeue();
        enqueued[s] = false;
        const auto sd = distance[s];
        // If we are using a shortest queue, no other path is going to be shorter
        // than f_distance at this point.

        if( ifst->Final(s).isZero() )
        {
            qDebug() << "kd_SingleShortestPath SHOULD NOT";
            exit(2);
        }

        for( fst::ArcIterator<fst::Fst<KdLatticeArc>> aiter(*ifst, s) ; !aiter.Done() ; aiter.Next() )
        {
            const auto &arc = aiter.Value();
            while (distance.size() <= arc.nextstate)
            {
                distance.push_back(KdLatticeWeight::Zero());
                enqueued.push_back(false);
                parent->emplace_back(KD_INVALID_STATE, KD_INVALID_ARC);
            }

            auto &nd = distance[arc.nextstate];
            const KdLatticeWeight weight = Times(sd, arc.weight);

            if( nd!=Plus(nd, weight) )
            {
                nd = Plus(nd, weight);
                if( !nd.isValid() )
                    return false;
                (*parent)[arc.nextstate] = std::make_pair(s, aiter.Position());
                if( !enqueued[arc.nextstate] )
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

void kd_fstShortestPath(KdLattice *ifst, KdLattice *ofst)
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

void kd_writeLat(KdLattice *ifst)
{
//    LatticeWriter lat_writer("ark:" KAL_SK_DIR "out.ark");
//    lat_writer.Write("f", *ifst);
//    fst::RemoveEpsLocal(ifst);
//    KdCompactLattice clat;
//    ConvertLattice(*ifst, &clat);

////////////TO WORK UNCOMMENT AND FIX THIS/////////////
//    KdCompactLWriter lat_writer("ark:" KAL_SK_DIR "out.ark");
//    lat_writer.Write("f", clat);

    // bool WriteCompactLattice(std::ostream &os, bool binary,
    // const CompactLattice &clat);
//    WriteCompactLattice(os, binary, clat);
///////////////////////////////////////////////////////
    if( ifst->NumStates()> 10 )
    {
        exit(0);
    }
}

fst::Fst<fst::StdArc> *kd_readDecodeGraph(char *filename)
{
    // read decoding network FST
    kaldi::Input ki(filename); // use ki.Stream() instead of is.
    if( !ki.Stream().good() )
    {
        qDebug() << "Could not open decoding-graph FST "
                 << filename;
    }

    fst::FstHeader hdr;
    if( !hdr.Read(ki.Stream(), "<unknown>") )
    {
        qDebug() << "Reading FST: error reading FST header.";
    }
    if( hdr.ArcType()!=fst::StdArc::Type() )
    {
        qDebug() << "FST with arc type " << hdr.ArcType().c_str() << " not supported.";
    }
    fst::FstReadOptions ropts("<unspecified>", &hdr);

    fst::Fst<fst::StdArc> *decode_fst = NULL;

    if( hdr.FstType()=="vector" )
    {
        decode_fst = fst::VectorFst<fst::StdArc>::Read(ki.Stream(), ropts);
    }
    else if( hdr.FstType()=="const" )
    {
        decode_fst = fst::ConstFst<fst::StdArc>::Read(ki.Stream(), ropts);
    }
    else
    {
        qDebug() << "Reading FST: unsupported FST type: "
                 << hdr.FstType().c_str();
    }
    if( decode_fst==NULL )
    {
        qDebug() << "Error reading FST (after reading header).";
        return NULL;
    }
    else
    {
        return decode_fst;
    }
}

void kd_ConvertLattice(KdLattice &ifst, fst::VectorFst<fst::StdArc> *ofst)
{
    ofst->DeleteStates();
    // The states will be numbered exactly the same as the original FST.
    // Add the states to the new FST.
    KdStateId num_states = ifst.NumStates();
    for( KdStateId s=0 ; s<num_states; s++ )
    {
        KdStateId news = ofst->AddState();
        assert(news==s);
    }
    ofst->SetStart(ifst.Start());
    for( KdStateId s = 0; s < num_states; s++)
    {
        KdLatticeWeight final_iweight = ifst.Final(s);
        if( final_iweight!=KdLatticeWeight::Zero() )
        {
            fst::StdArc::Weight final_oweight;
            ConvertLatticeWeight(final_iweight, &final_oweight);
            ofst->SetFinal(s, final_oweight);
        }
        for( fst::ArcIterator<fst::ExpandedFst<KdLatticeArc> > iter(ifst, s);
             !iter.Done();
             iter.Next())
        {
            KdLatticeArc arc = iter.Value();
            fst::StdArc oarc;
            ConvertLatticeWeight(arc.weight, &oarc.weight);
            oarc.ilabel = arc.ilabel;
            oarc.olabel = arc.olabel;
            oarc.nextstate = arc.nextstate;
            ofst->AddArc(s, oarc);
        }
    }
}
