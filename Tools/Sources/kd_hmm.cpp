#include "kd_hmm.h"
#include <QDebug>

using namespace kaldi;


void KdHmmTopology::Read(std::istream &is)
{
    ExpectToken(is, true, "<Topology>");
    ReadIntegerVector(is, true, &phones_);
    ReadIntegerVector(is, true, &phone2idx_);
    int32 sz;
    ReadBasicType(is, true, &sz);
    bool is_hmm = true;
    if (sz == -1) {
        is_hmm = false;
        ReadBasicType(is, true, &sz);
    }
    entries.resize(sz);
    for (int32 i = 0; i < sz; i++) {
        int32 thist_sz;
        ReadBasicType(is, true, &thist_sz);
        entries[i].resize(thist_sz);
        for (int32 j = 0 ; j < thist_sz; j++) {
            ReadBasicType(is, true, &(entries[i][j].forward_pdf_class));
            if (is_hmm)
                entries[i][j].self_loop_pdf_class = entries[i][j].forward_pdf_class;
            else
                ReadBasicType(is, true, &(entries[i][j].self_loop_pdf_class));
            int32 thiss_sz;
            ReadBasicType(is, true, &thiss_sz);
            entries[i][j].transitions.resize(thiss_sz);
            for (int32 k = 0; k < thiss_sz; k++) {
                ReadBasicType(is, true, &(entries[i][j].transitions[k].first));
                ReadBasicType(is, true, &(entries[i][j].transitions[k].second));
            }
        }
    }
    ExpectToken(is, true, "</Topology>");
}

KdHmmTopology::TopologyEntry& KdHmmTopology::TopologyForPhone(int32 phone)
{
    return entries[phone2idx_[phone]];
}
