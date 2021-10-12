#ifndef KD_FASTER_DECODER_H
#define KD_FASTER_DECODER_H

#include "decoder/faster-decoder.h"

typedef fst::Fst<fst::StdArc>             KdFST;

class KdFasterDecoder {
 public:
  typedef fst::StdArc Arc;
  typedef Arc::Label Label;
  typedef Arc::StateId StateId;
  typedef Arc::Weight Weight;

  KdFasterDecoder(KdFST &fst, kaldi::FasterDecoderOptions &config);
  ~KdFasterDecoder();

  /// Returns true if a final state was active on the last frame.
  bool ReachedFinal();

  /// GetBestPath gets the decoding traceback. If "use_final_probs" is true
  /// AND we reached a final state, it limits itself to final states;
  /// otherwise it gets the most likely token not taking into account
  /// final-probs. Returns true if the output best path was not the empty
  /// FST (will only return false in unusual circumstances where
  /// no tokens survived).
  bool GetBestPath(fst::MutableFst<LatticeArc> *fst_out,
                   bool use_final_probs = true);

  void InitDecoding();
  void Decode(kaldi::DecodableInterface *decodable);

  /// This will decode until there are no more frames ready in the decodable
  /// object, but if max_num_frames is >= 0 it will decode no more than
  /// that many frames.
  void AdvanceDecoding(kaldi::DecodableInterface *decodable,
                       int32 max_num_frames = -1);

  int32 NumFramesDecoded();
  void SetOptions(kaldi::FasterDecoderOptions &config);

 protected:

  class Token {
   public:
    Arc arc_; // contains only the graph part of the cost;
    // we can work out the acoustic part from difference between
    // "cost_" and prev->cost_.
    Token *prev_;
    int32 ref_count_;
    // if you are looking for weight_ here, it was removed and now we just have
    // cost_, which corresponds to ConvertToCost(weight_).
    double cost_;
    inline Token(const Arc &arc, BaseFloat ac_cost, Token *prev):
        arc_(arc), prev_(prev), ref_count_(1) {
      if (prev) {
        prev->ref_count_++;
        cost_ = prev->cost_ + arc.weight.Value() + ac_cost;
      } else {
        cost_ = arc.weight.Value() + ac_cost;
      }
    }
    inline Token(const Arc &arc, Token *prev):
        arc_(arc), prev_(prev), ref_count_(1) {
      if (prev) {
        prev->ref_count_++;
        cost_ = prev->cost_ + arc.weight.Value();
      } else {
        cost_ = arc.weight.Value();
      }
    }
    inline bool operator < (const Token &other) {
      return cost_ > other.cost_;
    }

    inline static void TokenDelete(Token *tok) {
      while (--tok->ref_count_ == 0) {
        Token *prev = tok->prev_;
        delete tok;
        if (prev == NULL) return;
        else tok = prev;
      }
    }
  };
  typedef kaldi::HashList<StateId, Token*>::Elem Elem;


  /// Gets the weight cutoff.  Also counts the active tokens.
  double GetCutoff(Elem *list_head, size_t *tok_count,
                   float *adaptive_beam, Elem **best_elem);

  void PossiblyResizeHash(size_t num_toks);

  // ProcessEmitting returns the likelihood cutoff used.
  // It decodes the frame num_frames_decoded_ of the decodable object
  // and then increments num_frames_decoded_
  double ProcessEmitting(kaldi::DecodableInterface *decodable);

  // TODO: first time we go through this, could avoid using the queue.
  void ProcessNonemitting(double cutoff);

  // HashList defined in ../util/hash-list.h.  It actually allows us to maintain
  // more than one list (e.g. for current and previous frames), but only one of
  // them at a time can be indexed by StateId.
  kaldi::HashList<StateId, Token*> toks_;
  fst::Fst<fst::StdArc> &fst_;
  kaldi::FasterDecoderOptions config_;
  std::vector<const Elem* > queue_;  // temp variable used in ProcessNonemitting,
  std::vector<float> tmp_array_;  // used in GetCutoff.
  // make it class member to avoid internal new/delete.

  // Keep track of the number of frames decoded in the current file.
  int32 num_frames_decoded_;

  // It might seem unclear why we call ClearToks(toks_.Clear()).
  // There are two separate cleanup tasks we need to do at when we start a new file.
  // one is to delete the Token objects in the list; the other is to delete
  // the Elem objects.  toks_.Clear() just clears them from the hash and gives ownership
  // to the caller, who then has to call toks_.Delete(e) for each one.  It was designed
  // this way for convenience in propagating tokens from one frame to the next.
  void ClearToks(Elem *list);
};


#endif // KD_FASTER_DECODER_H
