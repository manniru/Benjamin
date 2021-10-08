#ifndef KD_LATTICE_DECODER_H
#define KD_LATTICE_DECODER_H

#include "decoder/lattice-faster-decoder.h"

/*  extra_cost is used in pruning tokens, to save memory.

  extra_cost can be thought of as a beta (backward) cost assuming
  we had set the betas on currently-active tokens to all be the negative
  of the alphas for those tokens.  (So all currently active tokens would
  be on (tied) best paths).

  We can use the extra_cost to accurately prune away tokens that we know will
  never appear in the lattice.  If the extra_cost is greater than the desired
  lattice beam, the token would provably never appear in the lattice, so we can
  prune away the token.

  (Note: we don't update all the extra_costs every time we update a frame; we
  only do it every 'config_.prune_interval' frames).  */

typedef kaldi::decoder::StdToken             KdToken;
typedef fst::StdFst                          KdFST;
typedef fst::StdFst::Arc                     KdArc;
typedef KdArc::StateId                       KdStateId;
typedef kaldi::decoder::ForwardLink<KdToken> KdFLink;

// head of per-frame list of Tokens (list is in topological order),
// and something saying whether we ever pruned it using PruneForwardLinks.
struct KdTokenList
{
    KdToken *toks = NULL;
    bool must_prune_forward_links = true;
    bool must_prune_tokens = true;
};

class KdLatticeDecoder
{
public:
    using Label = typename KdArc::Label;
    using Weight = typename KdArc::Weight;

    kaldi::LatticeFasterDecoderConfig config_;

    KdLatticeDecoder(KdFST &fst, kaldi::LatticeFasterDecoderConfig &config);

    ~KdLatticeDecoder();

    /// Decodes until there are no more frames left in the "decodable" object
    /// Returns true if any kind of traceback is available
    bool Decode(kaldi::DecodableInterface *decodable);
    bool ReachedFinal();

    /// Returns true if result is nonempty.
    bool GetBestPath  (kaldi::Lattice *ofst, bool use_final_probs = true);
    bool GetRawLattice(kaldi::Lattice *ofst, bool use_final_probs = true);

    void InitDecoding();

    void AdvanceDecoding(kaldi::DecodableInterface *decodable,
                         int32 max_num_frames = -1);

    void  FinalizeDecoding();
    float FinalRelativeCost();
    int32 NumFramesDecoded();

protected:
    // protected instead of private, so classes which inherits from this,
    // also can have access

    inline static void DeleteForwardLinks(KdToken *tok);
    using Elem = typename kaldi::HashList<KdStateId, KdToken*>::Elem;

    void PossiblyResizeHash(size_t num_toks);

    inline Elem *FindOrAddToken(KdStateId state, int32 frame_plus_one,
                                float tot_cost, KdToken *backpointer,
                                bool *changed);

    void PruneForwardLinks(int32 frame_plus_one, bool *extra_costs_changed,
                           bool *links_pruned, float delta);

    void ComputeFinalCosts(unordered_map<KdToken*, float> *final_costs,
                           float *final_relative_cost,
                           float *final_best_cost);

    void PruneForwardLinksFinal();
    void PruneTokensForFrame(int32 frame_plus_one);
    void PruneActiveTokens(float delta);

    /// Gets the weight cutoff.  Also counts the active tokens.
    float GetCutoff(Elem *list_head, size_t *tok_count,
                    float *adaptive_beam, Elem **best_elem);

    /// Processes emitting arcs for one frame.  Propagates from prev_toks_ to
    /// cur_toks_.  Returns the cost cutoff for subsequent ProcessNonemitting() to
    /// use.
    float ProcessEmitting(kaldi::DecodableInterface *decodable);

    /// Processes nonemitting (epsilon) arcs for one frame.  Called after
    /// ProcessEmitting() on each frame.  The cost cutoff is computed by the
    /// preceding ProcessEmitting().
    void ProcessNonemitting(float cost_cutoff);

    // HashList defined in ../util/hash-list.h.  It actually allows us to maintain
    // more than one list (e.g. for current and previous frames), but only one of
    // them at a time can be indexed by KdStateId.  It is indexed by frame-index
    // plus one, where the frame-index is zero-based, as used in decodable object.
    // That is, the emitting probs of frame t are accounted for in tokens at
    // toks_[t+1].  The zeroth frame is for nonemitting transition at the start of
    // the graph.
    kaldi::HashList<KdStateId, KdToken*> toks_;

    std::vector<KdTokenList> active_toks_; // Lists of tokens, indexed by
    // frame (members of KdTokenList are toks, must_prune_forward_links,
    // must_prune_tokens).
    std::vector<Elem* > queue_;  // temp variable used in ProcessNonemitting,
    std::vector<float> tmp_array_;  // used in GetCutoff.

    // fst_ is a pointer to the FST we are decoding from.
    KdFST *fst_;

    std::vector<float> cost_offsets_; // This contains, for each
    // frame, an offset that was added to the acoustic log-likelihoods on that
    // frame in order to keep everything in a nice dynamic range i.e.  close to
    // zero, to reduce roundoff errors.
    int32 num_toks_; // current total #toks allocated...
    bool warned_;

    /// decoding_finalized_ is true if someone called FinalizeDecoding().
    /// calling this is optional.  If true, it's forbidden to decode more.
    bool decoding_finalized_;

    /// see ComputeFinalCosts().
    unordered_map<KdToken*, float> final_costs_;
    float final_relative_cost_;
    float final_best_cost_;

    void DeleteElems(Elem *list);

    static void TopSortTokens(KdToken *tok_list, std::vector<KdToken*> *topsorted_list);

    void ClearActiveTokens();
};


#endif // KD_LATTICE_DECODER_H
