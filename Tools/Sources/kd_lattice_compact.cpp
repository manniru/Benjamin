#include "kd_lattice_compact.h"

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

     for( int state=0 ; state<num_states ; state++ )
     {
         for( fst::ArcIterator<KdCompactLattice> aiter(lat, state) ;
              !aiter.Done() ; aiter.Next() )
         {
             const KdCompactLatticeArc &arc = aiter.Value();
             int state_start = kd_getStartTime(arc.weight.String());
             times[state] += state_start;
             int state_len = kd_getEndTime(arc.weight.String());
             state_len -= state_start;
             times.push_back(times[state] + state_len);
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
            const KdCompactLatticeArc &arc = aiter.Value();
            int arc_len = arc.weight.String().size();
            (*times)[arc.nextstate] = cur_time + arc_len;
        }
    }
}
