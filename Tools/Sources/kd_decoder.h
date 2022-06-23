#ifndef KD_DECODER_H
#define KD_DECODER_H

#include<QVector>
#include "kd_token.h"
#include "bt_token_list.h"
#include "kd_lattice_functions.h"
#include "kd_decodable.h"

// what is determinization?

#define MAX_STATE_COUNT 5000 //50K

struct KdDecoderConfig
{
    float beam = 16;
    int max_active = 900;
    int min_active = 200;
    float beam_delta = 0.5;
//    int prune_interval = 25;
//    float prune_scale = 0.1; // not a very important parameter.

    KdPrunedOpt det_opts;
};

// KdLatticeDecoder
class KdDecoder
{
public:
    KdDecoder();
    ~KdDecoder();

    void ResetDecoder();
    void InitDecoding(KdDecodable *dcodable);

    uint frame_num = 0; //number of decoded frame (Support up to 1Year)
    int  uframe;       // reset on ResetDecoder(utterance)
protected:
    // protected so classes which inherits also have access
    bool updateToken(KdStateId state, float  tot_cost,
                     KdToken **tok);

    float GetCutoff(KdToken **best_tok);
    double GetBestCutoff(KdToken *best_tok);

    float ProcessEmitting();
    float PEmittingState(KdToken *tok, float next_cutoff);
    void  ProcessNonemitting(float cost_cutoff);
    void  PNonemittingState(KdToken *tok, float cutoff);
    void  updateMaxState(KdStateId state);

    void DeleteTokArcs(KdToken *tok);
    // all tokens in current frame
    // map from state id to token
    KdToken    *cf_tokens[MAX_STATE_COUNT];
    KdDecodable *decodable;

    QVector<KdTokenList> frame_toks; // tokens indexed by frame

    // fst_ is a pointer to the FST we are decoding from.
    KdFST *fst_graph;

    KdDecoderConfig config;
    QVector<float> best_costs; //offset that keep costs close to
    // zero, to reduce roundoff errors.
    int max_state; // current total #toks allocated...
    bool warned_;

    bool decoding_finalized_; // true if someone called FinalizeDecoding().

    float final_relative_cost_;
    float final_best_cost_;
    float adaptive_beam; //updates in getcutoff

    void ResetCFToks();
    void ClearActiveTokens();
};


#endif // KD_DECODER_H
