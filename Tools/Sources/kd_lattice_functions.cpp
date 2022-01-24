#include "kd_lattice.h"

using namespace kaldi;

void kd_latticeGetTimes(Lattice *lat, std::vector<int> *times)
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
        for (fst::ArcIterator<Lattice> aiter(*lat, state); !aiter.Done();
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
