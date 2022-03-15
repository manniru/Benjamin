#include "kd_lattice_compact.h"
#include "fstext/factor.h"

#define MAX_SIL_COUNT 20

using namespace kaldi;

int kd_getStartTime(std::vector<int> input)
{
    int len = input.size();
    for( int i=0 ; i<len ; i++ )
    {
        if( input[i]>MAX_SIL_COUNT )
        {
            return i;
        }
    }

    return 0;
}

int kd_getEndTime(std::vector<int> input)
{
    int len = input.size();
    for( int i=len-1 ; i>0 ; i-- )
    {
        if( input[i]>MAX_SIL_COUNT )
        {
            return i;
        }
    }

    return 0;
}
// CompactLatticeStateTimes
QVector<int> kd_getTimes(KdCompactLattice &lat)
{
     int num_states = lat.NumStates();
     QVector<int> times;
     times.push_back(0);

     KdCLatArc arc;
     int state = 0;
     while( state<(num_states-1) )
     {
         for( fst::ArcIterator<KdCompactLattice> aiter(lat, state) ;
              !aiter.Done() ; aiter.Next() )
         {
             arc = static_cast<KdCLatArc>(aiter.Value());
             int state_start = kd_getStartTime(arc.weight.String());
             int state_len   = kd_getEndTime(arc.weight.String());
             if( state==0 )
             {
                 times[0]  += state_start;
                 state_len -= state_start;
                 state_len -= 1;
             }

             if( arc.weight.String().size()>1 )
             {
                 times.push_back(times.last() + state_len + 1);
             }

             state = arc.nextstate;
             break;
         }
     }

     return times;
}

// CompactLatticeStateTimes. use LatticeStateTimes for lattice
void kd_compactLatticeStateTimes(KdCompactLattice &lat,
                                std::vector<int> *times)
{
    if( !lat.Properties(fst::kTopSorted, true) )
    {
        KALDI_ERR << "Input lattice must be topologically sorted.";
    }
    KALDI_ASSERT( lat.Start()==0 );
    int num_states = lat.NumStates();
    times->clear();
    times->resize(num_states, -1);
    (*times)[0] = 0;
    for (int state = 0; state < num_states; state++)
    {
        int cur_time = (*times)[state];
        for (fst::ArcIterator<KdCompactLattice> aiter(lat, state); !aiter.Done();
             aiter.Next())
        {
            const KdCLatArc &arc = aiter.Value();
            int arc_len = arc.weight.String().size();
            (*times)[arc.nextstate] = cur_time + arc_len;
        }
    }
}

void ConvertLattice(const KdLattice &ifst, KdCompactLattice *ofst, bool invert)
{
    KdLattice ffst;
    std::vector<std::vector<int> > labels;
    if (invert) // normal case: want the ilabels as sequences on the arcs of
    {
        fst::Factor(ifst, &ffst, &labels);  // the output... Factor makes seqs of
    }
    // ilabels.
    else
    {
        KdLattice invfst(ifst);
        fst::Invert(&invfst);
        fst::Factor(invfst, &ffst,  &labels);
    }

    fst::TopSort(&ffst); // Put the states in ffst in topological order, which is
    // easier on the eye when reading the text-form lattices and corresponds to
    // what we get when we generate the lattices in the decoder.

    ofst->DeleteStates();

    // The states will be numbered exactly the same as the original FST.
    // Add the states to the new FST.
    KdStateId num_states = ffst.NumStates();
    for (KdStateId s = 0; s < num_states; s++)
    {
        KdStateId news = ofst->AddState();
        assert(news == s);
    }
    ofst->SetStart(ffst.Start());
    for (KdStateId s = 0; s < num_states; s++)
    {
        KdLatticeWeight final_weight = ffst.Final(s);
        if (final_weight != KdLatticeWeight::Zero())
        {
            KdCLatWeight final_compact_weight(final_weight, std::vector<int>());
            ofst->SetFinal(s, final_compact_weight);
        }
        for (fst::ArcIterator<fst::ExpandedFst<KdLatticeArc> > iter(ffst, s);
             !iter.Done();
             iter.Next())
        {
            const KdLatticeArc &arc = iter.Value();
            KALDI_PARANOID_ASSERT(arc.weight != Weight::Zero());
            // note: zero-weight arcs not allowed anyway so weight should not be zero,
            // but no harm in checking.
            KdCLatArc compact_arc(arc.olabel, arc.olabel,
                                  KdCLatWeight(arc.weight, labels[arc.ilabel]),
                    arc.nextstate);
            ofst->AddArc(s, compact_arc);
        }
    }
}
