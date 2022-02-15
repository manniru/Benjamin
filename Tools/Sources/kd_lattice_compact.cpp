#include "kd_lattice_compact.h"

using namespace kaldi;

// CompactLatticeStateTimes
int kd_compactLatticeStateTimes(KdCompactLattice &lat,
                                std::vector<int> *times)
{
    if (!lat.Properties(fst::kTopSorted, true))
        KALDI_ERR << "Input lattice must be topologically sorted.";
    KALDI_ASSERT(lat.Start() == 0);
    int num_states = lat.NumStates();
    times->clear();
    times->resize(num_states, -1);
    (*times)[0] = 0;
    int utt_len = -1;
    for (int state = 0; state < num_states; state++)
    {
        int cur_time = (*times)[state];
        for (fst::ArcIterator<KdCompactLattice> aiter(lat, state); !aiter.Done();
             aiter.Next())
        {
            const KdCompactLatticeArc &arc = aiter.Value();
            int arc_len = static_cast<int>(arc.weight.String().size());
            if ((*times)[arc.nextstate] == -1)
                (*times)[arc.nextstate] = cur_time + arc_len;
            else
                KALDI_ASSERT((*times)[arc.nextstate] == cur_time + arc_len);
        }
        if (lat.Final(state) != CompactLatticeWeight::Zero())
        {
            int this_utt_len = (*times)[state] + lat.Final(state).String().size();
            if (utt_len == -1) utt_len = this_utt_len;
            else
            {
                if (this_utt_len != utt_len)
                {
                    KALDI_WARN << "Utterance does not "
                                  "seem to have a consistent length.";
                    utt_len = std::max(utt_len, this_utt_len);
                }
            }
        }
    }
    if (utt_len == -1) {
        KALDI_WARN << "Utterance does not have a final-state.";
        return 0;
    }
    return utt_len;
}
