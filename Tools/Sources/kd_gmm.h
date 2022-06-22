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

    /// Returns the dimensionality of the Gaussian mean vectors
    int Dim() const { return means_invvars_.NumCols(); }

    float LogLikelihood(double *data, int len);

    /// Sets the gconsts.  Returns the number that are "invalid" e.g. because of
    /// zero weights or variances.
    void ComputeGconsts();

    void Read(std::istream &in);

private:
    float calcLogSum();

    /// Equals log(weight) - 0.5 * (log det(var) + mean*mean*inv(var))
    std::vector<float> gconsts;
    bool valid_gconsts_;   ///< Recompute gconsts_ if false
    std::vector<float> weights_;        ///< weights (not log).
    kaldi::Matrix<float> inv_vars_;       ///< Inverted (diagonal) variances
    kaldi::Matrix<float> means_invvars_;  ///< Means times inverted variance
    std::vector<float> loglikes;
};

#endif // KD_GMM_H
