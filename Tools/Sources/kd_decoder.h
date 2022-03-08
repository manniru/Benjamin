#ifndef KD_DECODER_H
#define KD_DECODER_H

#include<QVector>
#include "kd_token2.h"
#include "kd_lattice_functions.h"
#include "kd_decodable.h"
#include <util/hash-list.h>

// what is determinization?

#define KD_INFINITY std::numeric_limits<double>::infinity()
struct KdDecoderConfig
{
  float beam = 16;
  int32 max_active = 16000;
  int32 min_active = 200;
  float lattice_beam = 10.0;
  int32 prune_interval = 25;
  float beam_delta = 0.5;
  float hash_ratio = 2.0;
  float prune_scale = 0.1; // not a very important parameter.

  KdPrunedOpt det_opts;
};

struct KdTokenList
{
    KdToken2 *toks = NULL;
    bool prune_forward_links = true;
    bool prune_tokens = true;
};

// KdLatticeDecoder
class KdDecoder
{
public:
    typedef kaldi::HashList<KdStateId, KdToken2*>::Elem Elem;

    KdDecoderConfig config_;
    KdDecoder();
    ~KdDecoder();

    void InitDecoding(KdDecodable *dcodable);

    double GetBestCutoff(Elem *best_elem);

    long frame_num = 0; //number of decoded frame
protected:
    // protected instead of private, so classes which inherits from this,
    // also can have access
    inline static void DeleteForwardLinks(KdToken2 *tok);

    Elem *updateToken(KdStateId state, float  tot_cost,
                         bool *changed);

    void ComputeFinalCosts(unordered_map<KdToken2*, float> *final_costs,
                           float *final_relative_cost, float *final_best_cost);

    float GetCutoff(Elem *list_head, Elem **best_elem);

    float ProcessEmitting();
    float PEmittingElem(Elem *e, float next_cutoff);
    void  ProcessNonemitting(float cost_cutoff);
    void  PNonemittingElem(Elem *e, float cutoff);

    // Hash LinkList
    kaldi::HashList<KdStateId, KdToken2*> elements;
    KdDecodable *decodable;

    std::vector<KdTokenList> frame_toks; // Lists of tokens, indexed by
    // frame (members of KdTokenList are toks, must_prune_forward_links,
    // must_prune_tokens).
    std::vector<float> tmp_array_;  // used in GetCutoff.

    // fst_ is a pointer to the FST we are decoding from.
    KdFST *fst_;

    QVector<float> cost_offsets; //offset that keep costs close to
    // zero, to reduce roundoff errors.
    int num_elements; // current total #toks allocated...
    bool warned_;

    unordered_map<KdToken2*, float> final_costs_;
    float final_relative_cost_;
    float final_best_cost_;
    float adaptive_beam; //updates in getcutoff

    void DeleteElems(Elem *list);

    static void TopSortTokens(KdToken2 *tok_list,
                              std::vector<KdToken2*> *out);

    void ClearActiveTokens();
    void printActive();
};


#endif // KD_DECODER_H
