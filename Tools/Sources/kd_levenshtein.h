#ifndef KD_LEVENSHTEIN_H
#define KD_LEVENSHTEIN_H

#include <QObject>

#include "bt_mbr_base.h"
#include "kd_lattice_compact.h"
#include "kd_fst_util.h"

typedef struct KdGamma
{
    int   wid;  // word id
    float conf; // confidence level
}KdGamma;

struct GammaCompare
{
    // should be like operator <.  But we want reverse order
    // on the 2nd element (posterior), so it'll be like operator
    // > that looks first at the posterior.
    bool operator () (const KdGamma &a,
                      const KdGamma &b) const
    {
        if( a.conf>b.conf )
            return true;
        else if( a.conf<b.conf )
            return false;
        else
        {
            ///FIXME: wrong result
            return a.wid>b.wid;
        }
    }
};

typedef std::vector< std::vector<KdGamma> > KdGammaVec;

double kd_l_distance(int a, int b, bool penalize = false);
void kd_convertToVec(std::vector<std::map<int, double> > *map, KdGammaVec *out, int word_len);
void addToGamma(int wid, double conf, std::map<int, double> *map);

#endif // KD_LEVENSHTEIN_H
