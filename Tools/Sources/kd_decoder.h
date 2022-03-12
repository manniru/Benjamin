#ifndef KD_DECODER_H
#define KD_DECODER_H

#include<QVector>
#include "kd_token.h"
#include "kd_token_list.h"
#include "kd_lattice_functions.h"
#include "kd_decodable.h"

// what is determinization?

#define KD_INFINITY std::numeric_limits<double>::infinity()
#define MAX_STATE_COUNT 5000 //50K

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

// KdLatticeDecoder
class KdDecoder
{
public:
    KdDecoderConfig config_;
    KdDecoder();
    ~KdDecoder();

    void ResetDecoder();
    void InitDecoding(KdDecodable *dcodable);

    long frame_num = 0; //number of decoded frame
    int  uframe;       // reset on ResetDecoder(utterance)
protected:
    // protected so classes which inherits also have access
    bool updateToken(KdStateId state, float  tot_cost,
                     KdToken **tok);

    float GetCutoff(KdToken **best_tok);
    double GetBestCutoff(KdToken *tok);

    float ProcessEmitting();
    float PEmittingState(KdToken *tok, float next_cutoff);
    void  ProcessNonemitting(float cost_cutoff);
    void  PNonemittingState(KdToken *tok, float cutoff);
    void  updateMaxState(KdStateId state);

    void DeleteForwardLinks(KdToken *tok);
    // map from tokens in the current frame to state id
    KdToken    *all_tokens[MAX_STATE_COUNT];
    KdDecodable *decodable;

    QVector<KdTokenList> frame_toks; // tokens indexed by frame

    // fst_ is a pointer to the FST we are decoding from.
    KdFST *fst_;

    QVector<float> cost_offsets; //offset that keep costs close to
    // zero, to reduce roundoff errors.
    int max_state; // current total #toks allocated...
    bool warned_;

    bool decoding_finalized_; // true if someone called FinalizeDecoding().

    float final_relative_cost_;
    float final_best_cost_;
    float adaptive_beam; //updates in getcutoff

    void ClaerAllToks();
    void ClearActiveTokens();
};


#endif // KD_DECODER_H
