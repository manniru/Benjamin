#include "kd_cmvn_base.h"

using namespace kaldi;

void kd_applyCmvn(const MatrixBase<double> &stats,
               bool var_norm,
               MatrixBase<float> *feats)
{
    KALDI_ASSERT(feats != NULL);
    int32 dim = stats.NumCols() - 1;
    if (stats.NumRows() > 2 || stats.NumRows() < 1 || feats->NumCols() != dim)
    {
        KALDI_ERR << "Dim mismatch: cmvn "
                  << stats.NumRows() << 'x' << stats.NumCols()
                  << ", feats " << feats->NumRows() << 'x' << feats->NumCols();
    }
    if (stats.NumRows() == 1 && var_norm)
        KALDI_ERR << "You requested variance normalization but no variance stats "
                  << "are supplied.";

    double count = stats(0, dim);
    // Do not change the threshold of 1.0 here: in the balanced-cmvn code, when
    // computing an offset and representing it as stats, we use a count of one.
    if (count < 1.0)
        KALDI_ERR << "Insufficient stats for cepstral mean and variance normalization: "
                  << "count = " << count;

    if (!var_norm)
    {
        Vector<float> offset(dim);
        SubVector<double> mean_stats(stats.RowData(0), dim);
        offset.AddVec(-1.0 / count, mean_stats);
        feats->AddVecToRows(1.0, offset);
        return;
    }
    // norm(0, d) = mean offset;
    // norm(1, d) = scale, e.g. x(d) <-- x(d)*norm(1, d) + norm(0, d).
    Matrix<float> norm(2, dim);
    for (int32 d = 0; d < dim; d++)
    {
        double mean, offset, scale;
        mean = stats(0, d)/count;
        double var = (stats(1, d)/count) - mean*mean,
                floor = 1.0e-20;
        if (var < floor)
        {
            KALDI_WARN << "Flooring cepstral variance from " << var << " to "
                       << floor;
            var = floor;
        }
        scale = 1.0 / sqrt(var);
        if (scale != scale || 1/scale == 0.0)
            KALDI_ERR << "NaN or infinity in cepstral mean/variance computation";
        offset = -(mean*scale);
        norm(0, d) = offset;
        norm(1, d) = scale;
    }
    // Apply the normalization.
    feats->MulColsVec(norm.Row(1));
    feats->AddVecToRows(1.0, norm.Row(0));
}
