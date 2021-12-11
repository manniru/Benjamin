#ifndef KD_ONLINE_LDECODER_H
#define KD_ONLINE_LDECODER_H

#include <QVector>
#include "bt_config.h"

#ifdef BT_LAT_ONLINE

// This class provide a lattice online gmm decoder
// Kaldi only offer an online gmm decoder with no
// Lattice Support. Lattices Make it possible to
// get multi recognition output.

#include "util/stl-utils.h"
#include "kd_lattice_decoder.h"
#include "hmm/transition-model.h"
#include "lat/lattice-functions.h"
#include "kd_lattice.h"

#define KD_STATE_SILENCE 1
#define KD_STATE_NORMAL  2

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

class KdOnlineLDecoder : public KdLatticeDecoder
{
public:
    // "sil_phones" - the IDs of all silence phones
    KdOnlineLDecoder(fst::Fst<fst::StdArc> &fst, KdOnlineLDecoderOpts &opts,
                     QVector<int> sil_phones, kaldi::TransitionModel &trans_model);


    int Decode(kaldi::DecodableInterface *decodable);

    void RawLattice(int start, int end, kaldi::Lattice *ofst);
    void MakeLattice(int start, int end, kaldi::CompactLattice *ofst);

    // Makes a linear graph, by tracing back from the last "immortal" token
    // to the previous one
    bool PartialTraceback(kaldi::CompactLattice *out_fst);

    // Makes a linear graph, by tracing back from the best currently active token
    // to the last immortal token. This method is meant to be invoked at the end
    // of an utterance in order to get the last chunk of the hypothesis
    double FinishTraceBack(kaldi::CompactLattice *fst_out);

    // Returns "true" if the best current hypothesis ends with long enough silence
    bool HaveSilence();
    int32 frame; // the next frame to be processed

private:
    void ResetDecoder();
    bool GetiSymbol(kaldi::Lattice *fst, std::vector<int32> *isymbols_out);

    // Returns a linear fst by tracing back the last N frames, beginning
    // from the best current token
    void TracebackNFrames(int32 nframes, kaldi::Lattice *out_fst);
    KdToken2* getBestTok();

    // Searches for the last token, ancestor of all currently active tokens
    void UpdateImmortalToken();

    const KdOnlineLDecoderOpts opts_;
    QVector<int> silence_set; // silence phones IDs
    const kaldi::TransitionModel &trans_model_; // needed for trans-id -> phone conversion
    float max_beam_; // the maximum allowed beam
    float effective_beam_; // the currently used beam
    int   state_; // the current state of the decoder
    int32 utt_frames_; // # frames processed from the current utterance
    KdToken2 *immortal_tok_;      // "immortal" token means it's an ancestor of ...
    KdToken2 *prev_immortal_tok_; // ... all currently active tokens
};
#endif // BT_LAT_ONLINE

#endif // KD_ONLINE_DECODER_H
