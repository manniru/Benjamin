#ifndef KD_LATTICE_DECODER_H
#define KD_LATTICE_DECODER_H

#include<QVector>
#include "decoder/lattice-faster-decoder.h"
#include "kd_token2.h"
#include "kd_lattice.h"
#include "kd_online2_decodable.h"

#define KD_INFINITY std::numeric_limits<double>::infinity()

struct KdTokenList
{
    KdToken2 *toks = NULL;
    bool prune_forward_links = true;
    bool prune_tokens = true;
};

class KdLatticeDecoder
{
public:
    using Label = typename KdArc::Label;
    using Weight = typename KdArc::Weight;
    typedef kaldi::HashList<KdStateId, KdToken2*>::Elem Elem;

    kaldi::LatticeFasterDecoderConfig config_;
    KdLatticeDecoder();
    ~KdLatticeDecoder();

    void InitDecoding(KdOnline2Decodable *dcodable);
    void AdvanceDecoding();

    float FinalRelativeCost();

    double GetBestCutoff(Elem *best_elem);

    long frame_num = 0; //number of decoded frame

    void checkIntegrity(QString msg);
protected:
    // protected instead of private, so classes which inherits from this,
    // also can have access
    inline static void DeleteForwardLinks(KdToken2 *tok);

    Elem *FindOrAddToken(KdStateId state, float  tot_cost,
                         bool *changed);

    bool PruneForwardLinks(int32 frame, bool *extra_costs_changed, float delta);
    void ComputeFinalCosts(unordered_map<KdToken2*, float> *final_costs,
                           float *final_relative_cost, float *final_best_cost);

    void PruneTokensForFrame(int32 frame);
    void PruneActiveTokens(float delta);

    float GetCutoff(Elem *list_head, size_t *tok_count, Elem **best_elem);

    float ProcessEmitting();
    float PEmittingElem(Elem *e, float next_cutoff);
    void  ProcessNonemitting(float cost_cutoff);
    void  PNonemittingElem(Elem *e, float cutoff);

    // HashList is indexed by frame-index plus one.
    kaldi::HashList<KdStateId, KdToken2*> toks_;
    KdOnline2Decodable *decodable;

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

    unordered_map<KdToken2*, float> final_costs_;
    float final_relative_cost_;
    float final_best_cost_;
    float adaptive_beam; //updates in getcutoff

    void DeleteElems(Elem *list);

    static void TopSortTokens(KdToken2 *tok_list,
                              std::vector<KdToken2*> *topsorted_list);

    void ClearActiveTokens();
};


#endif // KD_LATTICE_DECODER_H
