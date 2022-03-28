#include "kd_cmvn_state.h"

using namespace kaldi;

KdRecyclingVector::KdRecyclingVector(int items_to_hold)
{
    max_size = items_to_hold;
    index_min = 0;
}

KdRecyclingVector::~KdRecyclingVector()
{
    for (auto *item : items_) {
        delete item;
    }
}

Vector<float> *KdRecyclingVector::At(int index)
{
    if( index<index_min )
    {
        KALDI_ERR << "failed to retrieve feature that was removed (index = "
                  << index << "; first_available_index = " << index_min;
    }
    // 'at' does size checking.
    return items_.at(index - index_min);
}

void KdRecyclingVector::PushBack(Vector<float> *item)
{
    if( items_.size()==max_size )
    { // need to remove
        delete items_.front();
        items_.pop_front();
        ++index_min;
    }
    items_.push_back(item);
}

int KdRecyclingVector::Size() const {
    return index_min + items_.size();
}
