#ifndef KD_FASTER_DECODER_H
#define KD_FASTER_DECODER_H

#define KD_INFINITY std::numeric_limits<double>::infinity()

#include "decoder/faster-decoder.h"
#include "kd_f_token.h"
#include "kd_lattice.h"

typedef fst::Fst<fst::StdArc> KdFST;

class KdFasterDecoder
{
public:
    typedef fst::StdArc Arc;
    typedef Arc::Label Label;
    typedef Arc::StateId StateId;
    typedef Arc::Weight Weight;

    KdFasterDecoder(KdFST &fst, kaldi::FasterDecoderOptions &config);
    ~KdFasterDecoder();

    bool ReachedFinal();

    bool GetBestPath(fst::MutableFst<KdLatticeArc> *fst_out,
                     bool use_final_probs = true);

    void InitDecoding();
    void Decode(kaldi::DecodableInterface *decodable);

    void AdvanceDecoding(kaldi::DecodableInterface *decodable,
                         int max_num_frames = -1);

    int NumFramesDecoded();
    void SetOptions(kaldi::FasterDecoderOptions &config);

protected:
    typedef kaldi::HashList<StateId, KdFToken*>::Elem Elem;

    double GetCutoff(Elem *list_head, size_t *tok_count,
                     float *adaptive_beam, Elem **best_elem);

    double GetBestCutoff(Elem *best_elem,
                         kaldi::DecodableInterface *decodable,
                         float adaptive_beam);

    double ProcessEmitting(kaldi::DecodableInterface *decodable);

    // TODO: first time we go through this, could avoid using the queue.
    void   ProcessNonemitting(double cutoff);

    // HashList defined in ../util/hash-list.h.  It actually allows us to maintain
    // more than one list (e.g. for current and previous frames), but only one of
    // them at a time can be indexed by StateId.
    kaldi::HashList<StateId, KdFToken*> toks_;
    fst::Fst<fst::StdArc> &fst_;
    kaldi::FasterDecoderOptions config_;
    std::vector<const Elem*> queue_;  // temp variable used in ProcessNonemitting,
    std::vector<float> tmp_array_;  // used in GetCutoff.
    // make it class member to avoid internal new/delete.

    // Keep track of the number of frames decoded in the current file.
    int num_frames_decoded_;

    void ClearToks(Elem *list);
};


#endif // KD_FASTER_DECODER_H
