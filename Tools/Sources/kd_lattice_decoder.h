#ifndef KD_LATTICE_DECODER_H
#define KD_LATTICE_DECODER_H

#include "decoder/lattice-faster-decoder.h"

//typedef KdLatticeDecoder<fst::StdFst, decoder::StdToken> LatticeFasterDecoder;
//template <typename FST, typename Token = decoder::StdToken>
typedef kaldi::decoder::StdToken KdToken;
typedef fst::StdFst              KdFST;
typedef fst::StdFst::Arc         KdArc;
typedef kaldi::decoder::StdToken KdToken;

class KdLatticeDecoder
{
public:
    using Arc = fst::StdFst::Arc;
    using Label = typename Arc::Label;
    using StateId = typename Arc::StateId;
    using Weight = typename Arc::Weight;
    using ForwardLinkT = decoder::ForwardLink<Token>;

    kaldi::LatticeFasterDecoderConfig config_;

    // This version does not take ownership of 'fst'.
    KdLatticeDecoder(KdFST &fst, LatticeFasterDecoderConfig &config);
    // But this version does and will delete it when this object is destroyed.
    KdLatticeDecoder(kaldi::LatticeFasterDecoderConfig &config, KdFST *fst);

    ~KdLatticeDecoder();

    /// Decodes until there are no more frames left in the "decodable" object
    /// Returns true if any kind of traceback is available
    bool Decode(DecodableInterface *decodable);
    bool ReachedFinal();

    /// Returns true if result is nonempty.
    bool GetBestPath  (kaldi::Lattice *ofst, bool use_final_probs = true);
    bool GetRawLattice(kaldi::Lattice *ofst, bool use_final_probs = true);

    void InitDecoding();

    void AdvanceDecoding(DecodableInterface *decodable,
                         int32 max_num_frames = -1);

    void  FinalizeDecoding();
    float FinalRelativeCost();
    int32 NumFramesDecoded();

protected:
    // protected instead of private, so classes which inherits from this,
    // also can have access

    inline static void DeleteForwardLinks(Token *tok);

    // head of per-frame list of Tokens (list is in topological order),
    // and something saying whether we ever pruned it using PruneForwardLinks.
    struct TokenList {
        Token *toks;
        bool must_prune_forward_links;
        bool must_prune_tokens;
        TokenList(): toks(NULL), must_prune_forward_links(true),
            must_prune_tokens(true) { }
    };

    using Elem = typename HashList<StateId, Token*>::Elem;
    // Equivalent to:
    //  struct Elem {
    //    StateId key;
    //    Token *val;
    //    Elem *tail;
    //  };

    void PossiblyResizeHash(size_t num_toks);

    // FindOrAddToken either locates a token in hash of toks_, or if necessary
    // inserts a new, empty token (i.e. with no forward links) for the current
    // frame.  [note: it's inserted if necessary into hash toks_ and also into the
    // singly linked list of tokens active on this frame (whose head is at
    // active_toks_[frame]).  The frame_plus_one argument is the acoustic frame
    // index plus one, which is used to index into the active_toks_ array.
    // Returns the Token pointer.  Sets "changed" (if non-NULL) to true if the
    // token was newly created or the cost changed.
    // If Token == StdToken, the 'backpointer' argument has no purpose (and will
    // hopefully be optimized out).
    inline Elem *FindOrAddToken(StateId state, int32 frame_plus_one,
                                BaseFloat tot_cost, Token *backpointer,
                                bool *changed);

    // prunes outgoing links for all tokens in active_toks_[frame]
    // it's called by PruneActiveTokens
    // all links, that have link_extra_cost > lattice_beam are pruned
    // delta is the amount by which the extra_costs must change
    // before we set *extra_costs_changed = true.
    // If delta is larger,  we'll tend to go back less far
    //    toward the beginning of the file.
    // extra_costs_changed is set to true if extra_cost was changed for any token
    // links_pruned is set to true if any link in any token was pruned
    void PruneForwardLinks(int32 frame_plus_one, bool *extra_costs_changed,
                           bool *links_pruned,
                           BaseFloat delta);

    // This function computes the final-costs for tokens active on the final
    // frame.  It outputs to final-costs, if non-NULL, a map from the Token*
    // pointer to the final-prob of the corresponding state, for all Tokens
    // that correspond to states that have final-probs.  This map will be
    // empty if there were no final-probs.  It outputs to
    // final_relative_cost, if non-NULL, the difference between the best
    // forward-cost including the final-prob cost, and the best forward-cost
    // without including the final-prob cost (this will usually be positive), or
    // infinity if there were no final-probs.  [c.f. FinalRelativeCost(), which
    // outputs this quanitity].  It outputs to final_best_cost, if
    // non-NULL, the lowest for any token t active on the final frame, of
    // forward-cost[t] + final-cost[t], where final-cost[t] is the final-cost in
    // the graph of the state corresponding to token t, or the best of
    // forward-cost[t] if there were no final-probs active on the final frame.
    // You cannot call this after FinalizeDecoding() has been called; in that
    // case you should get the answer from class-member variables.
    void ComputeFinalCosts(unordered_map<Token*, BaseFloat> *final_costs,
                           BaseFloat *final_relative_cost,
                           BaseFloat *final_best_cost) const;

    // PruneForwardLinksFinal is a version of PruneForwardLinks that we call
    // on the final frame.  If there are final tokens active, it uses
    // the final-probs for pruning, otherwise it treats all tokens as final.
    void PruneForwardLinksFinal();

    // Prune away any tokens on this frame that have no forward links.
    // [we don't do this in PruneForwardLinks because it would give us
    // a problem with dangling pointers].
    // It's called by PruneActiveTokens if any forward links have been pruned
    void PruneTokensForFrame(int32 frame_plus_one);


    // Go backwards through still-alive tokens, pruning them if the
    // forward+backward cost is more than lat_beam away from the best path.  It's
    // possible to prove that this is "correct" in the sense that we won't lose
    // anything outside of lat_beam, regardless of what happens in the future.
    // delta controls when it considers a cost to have changed enough to continue
    // going backward and propagating the change.  larger delta -> will recurse
    // less far.
    void PruneActiveTokens(BaseFloat delta);

    /// Gets the weight cutoff.  Also counts the active tokens.
    BaseFloat GetCutoff(Elem *list_head, size_t *tok_count,
                        BaseFloat *adaptive_beam, Elem **best_elem);

    /// Processes emitting arcs for one frame.  Propagates from prev_toks_ to
    /// cur_toks_.  Returns the cost cutoff for subsequent ProcessNonemitting() to
    /// use.
    BaseFloat ProcessEmitting(DecodableInterface *decodable);

    /// Processes nonemitting (epsilon) arcs for one frame.  Called after
    /// ProcessEmitting() on each frame.  The cost cutoff is computed by the
    /// preceding ProcessEmitting().
    void ProcessNonemitting(BaseFloat cost_cutoff);

    // HashList defined in ../util/hash-list.h.  It actually allows us to maintain
    // more than one list (e.g. for current and previous frames), but only one of
    // them at a time can be indexed by StateId.  It is indexed by frame-index
    // plus one, where the frame-index is zero-based, as used in decodable object.
    // That is, the emitting probs of frame t are accounted for in tokens at
    // toks_[t+1].  The zeroth frame is for nonemitting transition at the start of
    // the graph.
    HashList<StateId, Token*> toks_;

    std::vector<TokenList> active_toks_; // Lists of tokens, indexed by
    // frame (members of TokenList are toks, must_prune_forward_links,
    // must_prune_tokens).
    std::vector<const Elem* > queue_;  // temp variable used in ProcessNonemitting,
    std::vector<BaseFloat> tmp_array_;  // used in GetCutoff.

    // fst_ is a pointer to the FST we are decoding from.
    const KdFST *fst_;
    // delete_fst_ is true if the pointer fst_ needs to be deleted when this
    // object is destroyed.
    bool delete_fst_;

    std::vector<BaseFloat> cost_offsets_; // This contains, for each
    // frame, an offset that was added to the acoustic log-likelihoods on that
    // frame in order to keep everything in a nice dynamic range i.e.  close to
    // zero, to reduce roundoff errors.
    int32 num_toks_; // current total #toks allocated...
    bool warned_;

    /// decoding_finalized_ is true if someone called FinalizeDecoding().  [note,
    /// calling this is optional].  If true, it's forbidden to decode more.  Also,
    /// if this is set, then the output of ComputeFinalCosts() is in the next
    /// three variables.  The reason we need to do this is that after
    /// FinalizeDecoding() calls PruneTokensForFrame() for the final frame, some
    /// of the tokens on the last frame are freed, so we free the list from toks_
    /// to avoid having dangling pointers hanging around.
    bool decoding_finalized_;
    /// For the meaning of the next 3 variables, see the comment for
    /// decoding_finalized_ above., and ComputeFinalCosts().
    unordered_map<Token*, BaseFloat> final_costs_;
    BaseFloat final_relative_cost_;
    BaseFloat final_best_cost_;

    // There are various cleanup tasks... the toks_ structure contains
    // singly linked lists of Token pointers, where Elem is the list type.
    // It also indexes them in a hash, indexed by state (this hash is only
    // maintained for the most recent frame).  toks_.Clear()
    // deletes them from the hash and returns the list of Elems.  The
    // function DeleteElems calls toks_.Delete(elem) for each elem in
    // the list, which returns ownership of the Elem to the toks_ structure
    // for reuse, but does not delete the Token pointer.  The Token pointers
    // are reference-counted and are ultimately deleted in PruneTokensForFrame,
    // but are also linked together on each frame by their own linked-list,
    // using the "next" pointer.  We delete them manually.
    void DeleteElems(Elem *list);

    // This function takes a singly linked list of tokens for a single frame, and
    // outputs a list of them in topological order (it will crash if no such order
    // can be found, which will typically be due to decoding graphs with epsilon
    // cycles, which are not allowed).  Note: the output list may contain NULLs,
    // which the caller should pass over; it just happens to be more efficient for
    // the algorithm to output a list that contains NULLs.
    static void TopSortTokens(Token *tok_list,
                              std::vector<Token*> *topsorted_list);

    void ClearActiveTokens();

};


#endif // KD_LATTICE_DECODER_H
