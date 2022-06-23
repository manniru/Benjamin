#ifndef KD_GMM_H
#define KD_GMM_H

#include <utility>
#include <vector>

#include "bt_cfb.h"
#include "kd_io.h"

// QVector is slow during debug thus
// std vector is used

class KdGmm
{
public:
    KdGmm();

    float LogLikelihood(double *data, int len);
    void  ComputeGconsts();
    void  Read(std::istream &in);

private:
    float calcLogSum();

    /// Equals log(weight) - 0.5 * (log det(var) + mean*mean*inv(var))
    bool valid_gconsts_;   ///< Recompute gconsts_ if false
    KdMatrix inv_vars;       ///< Inverted (diagonal) variances
    KdMatrix means_invvars_;  ///< Means times inverted variance
    std::vector<float> gconsts;
    std::vector<float> weights_;        ///< weights (not log).
    std::vector<float> loglikes;
};

#endif // KD_GMM_H
