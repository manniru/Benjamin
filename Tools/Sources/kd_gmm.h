#ifndef KD_GMM_H
#define KD_GMM_H

#include <utility>
#include <vector>

#include "base/kaldi-common.h"
#include "matrix/matrix-lib.h"
#include "bt_cfb.h"

class KdGmm
{
public:
    KdGmm();

    /// Returns the dimensionality of the Gaussian mean vectors
    int Dim() const { return means_invvars_.NumCols(); }

    float LogLikelihood(kaldi::VectorBase<float> &data);

    /// Sets the gconsts.  Returns the number that are "invalid" e.g. because of
    /// zero weights or variances.
    void ComputeGconsts();

    void Read(std::istream &in);

private:
    float calcLogSum();

    /// Equals log(weight) - 0.5 * (log det(var) + mean*mean*inv(var))
    QVector<float> gconsts;
    bool valid_gconsts_;   ///< Recompute gconsts_ if false
    QVector<float> weights_;        ///< weights (not log).
    kaldi::Matrix<float> inv_vars_;       ///< Inverted (diagonal) variances
    kaldi::Matrix<float> means_invvars_;  ///< Means times inverted variance
    QVector<float> loglikes;
};

#endif // KD_GMM_H
