#include "kd_clat_weight.h"
#include <QDebug>


KdCLatWeight::KdCLatWeight()
{

}

KdCLatWeight::KdCLatWeight(KdLatticeWeight w, std::vector<int> s)
{
    weight = w;
    string = s;
}

const KdCLatWeight KdCLatWeight::Zero()
{
    return KdCLatWeight(KdLatticeWeight::Zero(), std::vector<int>());
}

const KdCLatWeight KdCLatWeight::One()
{
    return KdCLatWeight(
                KdLatticeWeight::One(), std::vector<int>());
}

string KdCLatWeight::Type()
{
    char buf[2];
    buf[0] = '0' + sizeof(int);
    buf[1] = '\0';
    static const std::string type = "compact" + KdLatticeWeight::Type()
            + buf;
    return type;
}

int Compare(const KdCLatWeight &w1, const KdCLatWeight &w2)
{
    int c1 = Compare(w1.weight, w2.weight);
    if (c1!=0)
        return c1;
    int l1 = w1.string.size(), l2 = w2.string.size();
    // Use opposite order on the string lengths, so that if the costs are the same,
    // the shorter string wins.
    if (l1 > l2)
        return -1;
    else if (l1 < l2)
        return 1;
    for(int i = 0; i < l1; i++)
    {
        if (w1.string[i] < w2.string[i])
            return -1;
        else if (w1.string[i] > w2.string[i])
            return 1;
    }
    return 0;
}
