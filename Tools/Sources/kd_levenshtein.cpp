#include "kd_levenshtein.h"
#include <QDebug>

#define KD_MBR_DELTA 1.0e-05 // used for l function

void kd_convertToVec(std::vector<std::map<int, double> > *map, KdGammaVec *out, int word_len)
{
    out->clear();
    out->resize(word_len);

    for (int q = 1; q<=word_len ; q++ )
    {
        for( std::map<int, double>::iterator iter = (*map)[q].begin();
             iter!=(*map)[q].end(); ++iter)
        {
            KdGamma g_buf;
            g_buf.wid = iter->first;
            g_buf.conf = iter->second;
            (*out)[q-1].push_back(g_buf);
        }
        // sort gamma_[q-1] from largest to smallest posterior.
        GammaCompare comp;
        std::sort((*out)[q-1].begin(), (*out)[q-1].end(), comp);
    }
}

void addToGamma(int wid, double conf, std::map<int, double> *map)
{
    if( conf==0 )
        return;
    std::pair<const int, double> pr(wid, conf);
    std::pair<std::map<int, double>::iterator, bool> ret = map->insert(pr);
    if( !ret.second ) // not inserted, so add to contents.
        ret.first->second += conf;
}

double kd_l_distance(int a, int b, bool penalize)
{
    if( a==b)
    {
        return 0.0;
    }
    else if( penalize )
    {
        return 1.0 + KD_MBR_DELTA;
    }
    else
    {
        return 1.0;
    }
}
