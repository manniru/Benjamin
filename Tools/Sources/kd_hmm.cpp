#include "kd_hmm.h"
#include <QDebug>

void KdHmmTopology::Read(std::istream &is)
{
    kd_ExpectToken(is, "<Topology>");
    kd_ReadIntegerVector(is, &phones_);
    kd_ReadIntegerVector(is, &phone2idx_);
    int sz;
    kd_ReadBasicType(is, &sz);
    bool is_hmm = true;
    if( sz == -1) {
        is_hmm = false;
        kd_ReadBasicType(is, &sz);
    }
    entries.resize(sz);
    for( int i = 0; i < sz; i++ )
    {
        int thist_sz;
        kd_ReadBasicType(is, &thist_sz);
        entries[i].resize(thist_sz);
        for( int j = 0 ; j < thist_sz; j++ )
        {
            kd_ReadBasicType(is, &(entries[i][j].forward_pdf_class));
            if( is_hmm)
                entries[i][j].self_loop_pdf_class = entries[i][j].forward_pdf_class;
            else
                kd_ReadBasicType(is, &(entries[i][j].self_loop_pdf_class));
            int thiss_sz;
            kd_ReadBasicType(is, &thiss_sz);
            entries[i][j].transitions.resize(thiss_sz);
            for( int k = 0; k < thiss_sz; k++ )
            {
                kd_ReadBasicType(is, &(entries[i][j].transitions[k].first));
                kd_ReadBasicType(is, &(entries[i][j].transitions[k].second));
            }
        }
    }
    kd_ExpectToken(is, "</Topology>");
}

KdHmmTopology::TopologyEntry& KdHmmTopology::TopologyForPhone(int phone)
{
    return entries[phone2idx_[phone]];
}
