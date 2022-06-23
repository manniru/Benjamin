#ifndef KD_LATTICE_STRING_H
#define KD_LATTICE_STRING_H

#include<QVector>
#include "kd_lattice.h"
#include "kd_clat_weight.h"


// This class maps back and forth from/to integer id's to sequences of strings.
// used in determinization algorithm.  It is constructed in such a way that
// finding the string-id of the successor of (string, next-label) has constant time.

// Note: class IntType, typically int, is the type of the element in the
// string (typically a template argument of the CompactLatticeWeightTpl).

// KdLatString
class KdLatString
{
public:
    struct Entry
    {
        const Entry *parent; // NULL for empty string.
        int i;
        inline bool operator==(const Entry &other) const
        {
            return (parent==other.parent && i==other.i);
        }
        Entry() { }
        Entry(const Entry &e): parent(e.parent), i(e.i) {}
    };
    // Note: all Entry* pointers returned in function calls are
    // owned by the repository itself, not by the caller!

    // Interface guarantees empty string is NULL.
    inline const Entry *EmptyString() { return NULL; }

    // Returns string of "parent" with i appended.  Pointer
    // owned by repository
    const Entry *Successor(const Entry *parent, int i) {
        new_entry_->parent = parent;
        new_entry_->i = i;

        std::pair<typename SetType::iterator, bool> pr = set_.insert(new_entry_);
        if( pr.second) { // Was successfully inserted (was not there).  We need to
            // replace the element we inserted, which resides on the
            // stack, with one from the heap.
            const Entry *ans = new_entry_;
            new_entry_ = new Entry();
            return ans;
        } else { // Was not inserted because an equivalent Entry already
            // existed.
            return *pr.first;
        }
    }

    const Entry *Concatenate (const Entry *a, const Entry *b) {
        if( a==NULL) return b;
        else if( b==NULL) return a;
        std::vector<int> v;
        ConvertToVector(b, &v);
        const Entry *ans = a;
        for(size_t i = 0; i < v.size(); i++)
            ans = Successor(ans, v[i]);
        return ans;
    }
    const Entry *CommonPrefix (const Entry *a, const Entry *b) {
        std::vector<int> a_vec, b_vec;
        ConvertToVector(a, &a_vec);
        ConvertToVector(b, &b_vec);
        const Entry *ans = NULL;
        for(size_t i = 0; i < a_vec.size() && i < b_vec.size() &&
            a_vec[i]==b_vec[i]; i++)
            ans = Successor(ans, a_vec[i]);
        return ans;
    }

    // removes any elements from b that are not part of
    // a common prefix with a.
    void ReduceToCommonPrefix(const Entry *a,
                              std::vector<int> *b) {
        size_t a_size = Size(a), b_size = b->size();
        while (a_size> b_size) {
            a = a->parent;
            a_size--;
        }
        if( b_size > a_size)
            b_size = a_size;
        typename std::vector<int>::iterator b_begin = b->begin();
        while (a_size!=0) {
            if( a->i!=*(b_begin + a_size - 1))
                b_size = a_size - 1;
            a = a->parent;
            a_size--;
        }
        if( b_size!=b->size())
            b->resize(b_size);
    }

    // removes the first n elements of a.
    const Entry *RemovePrefix(const Entry *a, size_t n) {
        if( n==0) return a;
        std::vector<int> a_vec;
        ConvertToVector(a, &a_vec);
        assert(a_vec.size() >= n);
        const Entry *ans = NULL;
        for(size_t i = n; i < a_vec.size(); i++)
            ans = Successor(ans, a_vec[i]);
        return ans;
    }



    // Returns true if a is a prefix of b.  If a is prefix of b,
    // time taken is |b| - |a|.  Else, time taken is |b|.
    bool IsPrefixOf(const Entry *a, const Entry *b) const {
        if(a==NULL) return true; // empty string prefix of all.
        if( a==b) return true;
        if( b==NULL) return false;
        return IsPrefixOf(a, b->parent);
    }


    inline size_t Size(const Entry *entry) const {
        size_t ans = 0;
        while (entry!=NULL) {
            ans++;
            entry = entry->parent;
        }
        return ans;
    }

    void ConvertToVector(const Entry *entry, std::vector<int> *out) const {
        size_t length = Size(entry);
        out->resize(length);
        if( entry!=NULL) {
            typename std::vector<int>::reverse_iterator iter = out->rbegin();
            while (entry!=NULL) {
                *iter = entry->i;
                entry = entry->parent;
                ++iter;
            }
        }
    }

    const Entry *ConvertFromVector(const std::vector<int> &vec) {
        const Entry *e = NULL;
        for(size_t i = 0; i < vec.size(); i++)
            e = Successor(e, vec[i]);
        return e;
    }

    KdLatString() { new_entry_ = new Entry; }

    void Destroy() {
        for( typename SetType::iterator iter = set_.begin();
             iter!=set_.end();
             ++iter)
            delete *iter;
        SetType tmp;
        tmp.swap(set_);
        if( new_entry_) {
            delete new_entry_;
            new_entry_ = NULL;
        }
    }

    // Rebuild will rebuild this object, guaranteeing only
    // to preserve the Entry values that are in the vector pointed
    // to (this list does not have to be unique).  The point of
    // this is to save memory.
    void Rebuild(const std::vector<const Entry*> &to_keep) {
        SetType tmp_set;
        for( typename std::vector<const Entry*>::const_iterator
             iter = to_keep.begin();
             iter!=to_keep.end(); ++iter)
            RebuildHelper(*iter, &tmp_set);
        // Now delete all elems not in tmp_set.
        for( typename SetType::iterator iter = set_.begin();
             iter!=set_.end(); ++iter) {
            if( tmp_set.count(*iter)==0)
                delete (*iter); // delete the Entry; not needed.
        }
        set_.swap(tmp_set);
    }

    ~KdLatString() { Destroy(); }
    int MemSize() const {
        return set_.size() * sizeof(Entry) * 2; // this is a lower bound
        // on the size this structure might take.
    }
private:
    class EntryKey { // Hash function object.
    public:
        inline size_t operator()(const Entry *entry) const {
            size_t prime = 49109;
            return static_cast<size_t>(entry->i)
                    + prime * reinterpret_cast<size_t>(entry->parent);
        }
    };
    class EntryEqual {
    public:
        inline bool operator()(const Entry *e1, const Entry *e2) const {
            return (*e1==*e2);
        }
    };
    typedef std::unordered_set<const Entry*, EntryKey, EntryEqual> SetType;

    void RebuildHelper(const Entry *to_add, SetType *tmp_set) {
        while(true) {
            if( to_add==NULL) return;
            typename SetType::iterator iter = tmp_set->find(to_add);
            if( iter==tmp_set->end()) { // not in tmp_set.
                tmp_set->insert(to_add);
                to_add = to_add->parent; // and loop.
            } else {
                return;
            }
        }
    }

    Entry *new_entry_; // We always have a pre-allocated Entry ready to use,
    // to avoid unnecessary news and deletes.
    SetType set_;

};


#endif // KD_LATTICE_STRING_H
