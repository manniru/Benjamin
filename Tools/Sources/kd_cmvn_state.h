#ifndef KD_CMVN_STATE_H
#define KD_CMVN_STATE_H

#include <QObject>
#include <QThread>

#include <matrix/kaldi-matrix.h>
#include <deque>

struct KdCmvnState  //OnlineCmvnState
{
    // total CMVN stats for this speaker (up till now)
    kaldi::Matrix<double> speaker_cmvn_stats;

    // format, of [     sum-stats      count
    //              sum-squared-stats    0    ]
    kaldi::Matrix<double> global_cmvn_stats;
    kaldi::Matrix<double> frozen_state;
};

/// This class serves as a storage for feature vectors with an option to limit
/// the memory usage by removing old elements.
class KdRecyclingVector
{
public:
    /// By default it does not remove any elements.
    KdRecyclingVector(int items_to_hold = -1);

    /// The ownership is being retained by this collection - do not delete the item.
    kaldi::Vector<float> *At(int index) const;

    /// The ownership of the item is passed to this collection - do not delete the item.
    void PushBack(kaldi::Vector<float> *item);

    /// This method returns the size as if no "recycling" had happened,
    /// i.e. equivalent to the number of times the PushBack method has been called.
    int Size() const;

    ~KdRecyclingVector();

private:
    std::deque<kaldi::Vector<float>*> items_;
    int items_to_hold_;
    int first_available_index_;
};

#endif // KD_CMVN_STATE_H
