#include "kd_cmvn_state.h"

using namespace kaldi;

KdRecyclingVector::KdRecyclingVector(int items_to_hold):
    items_to_hold_(items_to_hold == 0 ? -1 : items_to_hold),
    first_available_index_(0) {
}

KdRecyclingVector::~KdRecyclingVector()
{
    for (auto *item : items_) {
        delete item;
    }
}

Vector<float> *KdRecyclingVector::At(int index) const
{
    if (index < first_available_index_) {
        KALDI_ERR << "Attempted to retrieve feature vector that was "
                     "already removed by the KdRecyclingVector (index = "
                  << index << "; "
                  << "first_available_index = " << first_available_index_ << "; "
                  << "size = " << Size() << ")";
    }
    // 'at' does size checking.
    return items_.at(index - first_available_index_);
}

void KdRecyclingVector::PushBack(Vector<float> *item)
{
    if (items_.size() == items_to_hold_) {
        delete items_.front();
        items_.pop_front();
        ++first_available_index_;
    }
    items_.push_back(item);
}

int KdRecyclingVector::Size() const {
    return first_available_index_ + items_.size();
}
