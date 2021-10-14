#ifndef KD_ONLINE_DECODER_H
#define KD_ONLINE_DECODER_H

#include "bt_config.h"

#ifndef BT_LAT_ONLINE

#include "util/stl-utils.h"
#include "decoder/faster-decoder.h"
#include "hmm/transition-model.h"

#define KD_INFINITY std::numeric_limits<double>::infinity()


struct KdOnlineDecoderOpts: public kaldi::FasterDecoderOptions
{
    float rt_min = 0.7;          // minimum decoding runtime factor
    float rt_max = 0.75;         // maximum decoding runtime factor
    int32 batch_size = 27;       // number of features decoded in one go
    int32 inter_utt_sil = 50;    // minimum silence (#frames) to trigger end of utterance
    int32 max_utt_len_  = 1500;  // if utt. is longer, we accept shorter silence as utt. separators
    int32 update_interval = 3;   // beam update period in # of frames
    float beam_update = 0.01;    // rate of adjustment of the beam
    float max_beam_update = 0.05;// maximum rate of beam adjustment
};

// Codes returned by Decode() to show the current state of the decoder
enum KdDecodeState
{
    KD_EndFeats = 1,// No more scores are available from the Decodable
    KD_EndUtt   = 2,// End of utterance, caused by e.g. a sufficiently long silence
    KD_EndBatch = 4 // End of batch - end of utterance not reached yet
};

class KdOnlineDecoder : public kaldi::FasterDecoder
{
public:
    // "sil_phones" - the IDs of all silence phones
    KdOnlineDecoder(const fst::Fst<fst::StdArc> &fst,
                     const KdOnlineDecoderOpts &opts,
                     const std::vector<int32> &sil_phones,
                     const kaldi::TransitionModel &trans_model);


    KdDecodeState Decode(kaldi::DecodableInterface *decodable);

    int  PartialTraceback(fst::MutableFst<kaldi::LatticeArc> *out_fst);
    int  FinishTraceBack (fst::MutableFst<kaldi::LatticeArc> *out_fst);

    // Returns "true" if the best current hypothesis ends with long enough silence
    bool EndOfUtterance();

private:
    void ResetDecoder(bool full);

    // Returns a linear fst by tracing back the last N frames, beginning
    // from the best current token
    void TracebackNFrames(int32 nframes,
                          fst::MutableFst<kaldi::LatticeArc> *out_fst);

    // Makes a linear "lattice", by tracing back a path delimited by two tokens
    void MakeLattice(Token *start, Token *end,
                     fst::MutableFst<kaldi::LatticeArc> *out_fst);

    // Searches for the last token, ancestor of all currently active tokens
    void   UpdateImmortalToken();

    double getConf(Token *best_tok);
    Token *getBestTok();
    double updateBeam(double tstart);

    KdOnlineDecoderOpts opts_;
    const kaldi::ConstIntegerSet<int32> silence_set_; // silence phones IDs
    const kaldi::TransitionModel &trans_model_; // needed for trans-id -> phone conversion
    float max_beam_; // the maximum allowed beam
    float effective_beam_; // the currently used beam
    KdDecodeState state_; // the current state of the decoder
    int32 frame_; // the next frame to be processed
    int32 utt_frames_; // # frames processed from the current utterance
    Token *immortal_tok_;      // "immortal" token means it's an ancestor of ...
    Token *prev_immortal_tok_; // ... all currently active tokens
};

#endif // BT_LAT_ONLINE

#endif // KD_ONLINE_DECODER_H
