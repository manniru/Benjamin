#ifndef KD_LATTICE_DECODER_H
#define KD_LATTICE_DECODER_H

#include<QVector>
#include "decoder/lattice-faster-decoder.h"

/*extra_cost is used in pruning tokens, to save memory.

  extra_cost can be thought of as a beta (backward) cost assuming
  we had set the betas on currently-active tokens to all be the negative
  of the alphas for those tokens.  (So all currently active tokens would
  be on (tied) best paths).

  We can use the extra_cost to accurately prune away tokens that we know will
  never appear in the lattice.  If the extra_cost is greater than the desired
  lattice beam, the token would provably never appear in the lattice, so we can
  prune away the token.

  (Note: we don't update all the extra_costs every time we update a frame; we
  only do it every 'config_.prune_interval' frames).*/

typedef kaldi::decoder::StdToken             KdToken;
typedef fst::StdFst                          KdFST;
typedef fst::StdFst::Arc                     KdArc;
typedef KdArc::StateId                       KdStateId;
typedef kaldi::decoder::ForwardLink<KdToken> KdFLink;

#define KD_INFINITY std::numeric_limits<double>::infinity()

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
    typedef kaldi::HashList<KdStateId, KdToken*>::Elem Elem;

    kaldi::LatticeFasterDecoderConfig config_;
    KdLatticeDecoder(KdFST &fst, kaldi::LatticeFasterDecoderConfig &config);
    ~KdLatticeDecoder();

    void InitDecoding();
    bool Decode(kaldi::DecodableInterface *decodable);
    void AdvanceDecoding(kaldi::DecodableInterface *decodable);

    void  FinalizeDecoding();
    float FinalRelativeCost();

    bool GetBestPath  (kaldi::Lattice *ofst, bool use_final_probs = true);
    bool GetRawLattice(kaldi::Lattice *ofst, bool use_final_probs = true);
    double GetBestCutoff(Elem *best_elem,
                         kaldi::DecodableInterface *decodable,
                         float adaptive_beam);

    long frame_num = 0; //number of decoded frame

protected:
    // protected instead of private, so classes which inherits from this,
    // also can have access
    inline static void DeleteForwardLinks(KdToken *tok);

    void PossiblyResizeHash(size_t num_toks);
    inline Elem *FindOrAddToken(KdStateId state, int32 frame_plus_one,
                                float tot_cost, KdToken *backpointer,
                                bool *changed);

    bool PruneForwardLinks(int32 frame, bool *extra_costs_changed, float delta);
    void ComputeFinalCosts(unordered_map<KdToken*, float> *final_costs,
                           float *final_relative_cost, float *final_best_cost);

    void PruneForwardLinksFinal();
    void PruneTokensForFrame(int32 frame);
    void PruneActiveTokens(float delta);

    float GetCutoff(Elem *list_head, size_t *tok_count,
                    float *adaptive_beam, Elem **best_elem);

    float ProcessEmitting(kaldi::DecodableInterface *decodable);
    void ProcessNonemitting(float cost_cutoff);

    // HashList is indexed by frame-index plus one.
    kaldi::HashList<KdStateId, KdToken*> toks_;

    std::vector<KdTokenList> frame_toks; // Lists of tokens, indexed by
    // frame (members of KdTokenList are toks, must_prune_forward_links,
    // must_prune_tokens).
    std::vector<Elem* > queue_;  // temp variable used in ProcessNonemitting,
    std::vector<float> tmp_array_;  // used in GetCutoff.

    // fst_ is a pointer to the FST we are decoding from.
    KdFST *fst_;

    QVector<float> cost_offsets; //offset that keep costs close to
    // zero, to reduce roundoff errors.
    int32 num_toks_; // current total #toks allocated...
    bool warned_;

    bool decoding_finalized_; // true if someone called FinalizeDecoding().

    unordered_map<KdToken*, float> final_costs_;
    float final_relative_cost_;
    float final_best_cost_;

    void DeleteElems(Elem *list);

    static void TopSortTokens(KdToken *tok_list, std::vector<KdToken*> *topsorted_list);

    void ClearActiveTokens();
};


#endif // KD_LATTICE_DECODER_H
