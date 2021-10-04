#ifndef KD_ONLINE_DECODER_H
#define KD_ONLINE_DECODER_H

// This class provide a lattice online gmm decoder
// Kaldi only offer an online gmm decoder with no
// Lattice Support. Lattices Make it possible to
// get multi recognition output.

#include "util/stl-utils.h"
#include "decoder/lattice-faster-decoder.h"
#include "hmm/transition-model.h"

typedef kaldi::decoder::StdToken KdToken;

struct KdOnlineLDecoderOpts: public kaldi::LatticeFasterDecoderConfig
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

class KdOnlineLDecoder : public kaldi::LatticeFasterDecoder
{
public:
    // "sil_phones" - the IDs of all silence phones
    KdOnlineLDecoder(const fst::Fst<fst::StdArc> &fst,
                     const KdOnlineLDecoderOpts &opts,
                     const std::vector<int32> &sil_phones,
                     const kaldi::TransitionModel &trans_model);


    KdDecodeState Decode(kaldi::DecodableInterface *decodable);

    // Makes a linear graph, by tracing back from the last "immortal" token
    // to the previous one
    bool PartialTraceback(fst::MutableFst<kaldi::LatticeArc> *out_fst);

    // Makes a linear graph, by tracing back from the best currently active token
    // to the last immortal token. This method is meant to be invoked at the end
    // of an utterance in order to get the last chunk of the hypothesis
    void FinishTraceBack(fst::MutableFst<kaldi::LatticeArc> *fst_out);

    // Returns "true" if the best current hypothesis ends with long enough silence
    bool EndOfUtterance();

    int32 frame() { return frame_; }

private:
    void ResetDecoder(bool full);

    // Returns a linear fst by tracing back the last N frames, beginning
    // from the best current token
    void TracebackNFrames(int32 nframes,
                          fst::MutableFst<kaldi::LatticeArc> *out_fst);

    // Makes a linear "lattice", by tracing back a path delimited by two tokens
    void MakeLattice(KdToken *start, KdToken *end,
                     kaldi::Lattice *out_fst);

    // Searches for the last token, ancestor of all currently active tokens
    void UpdateImmortalToken();

    const KdOnlineLDecoderOpts opts_;
    const kaldi::ConstIntegerSet<int32> silence_set_; // silence phones IDs
    const kaldi::TransitionModel &trans_model_; // needed for trans-id -> phone conversion
    float max_beam_; // the maximum allowed beam
    float effective_beam_; // the currently used beam
    float state_; // the current state of the decoder
    int32 frame_; // the next frame to be processed
    int32 utt_frames_; // # frames processed from the current utterance
    KdToken *immortal_tok_;      // "immortal" token means it's an ancestor of ...
    KdToken *prev_immortal_tok_; // ... all currently active tokens
};

#endif // KD_ONLINE_DECODER_H
