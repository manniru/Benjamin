#include "kd_cmvn_base.h"

using namespace kaldi;

// only apply mean
void kd_applyCmvn(const MatrixBase<double> &stats,
               MatrixBase<float> *feats)
{
    KALDI_ASSERT(feats != NULL);
    int32 dim = stats.NumCols() - 1;
    if( stats.NumRows() > 2 || stats.NumRows() < 1 || feats->NumCols() != dim)
    {
        KALDI_ERR << "Dim mismatch: cmvn "
                  << stats.NumRows() << 'x' << stats.NumCols()
                  << ", feats " << feats->NumRows() << 'x' << feats->NumCols();
    }

    double count = stats(0, dim);
    // Do not change the threshold of 1.0 here: in the balanced-cmvn code, when
    // computing an offset and representing it as stats, we use a count of one.
    if( count < 1.0)
        KALDI_ERR << "Insufficient stats for cepstral mean and variance normalization: "
                  << "count = " << count;

    Vector<float> offset(dim);
    SubVector<double> mean_stats(stats.RowData(0), dim);
    offset.AddVec(-1.0 / count, mean_stats);
    feats->AddVecToRows(1.0, offset);
    return;
}
