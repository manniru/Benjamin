#ifndef KD_ONLINE_LDECODER_H
#define KD_ONLINE_LDECODER_H

#include <QVector>
#include "bt_config.h"
#include "backend.h"

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
#include "kd_lattice_functions.h"
#include "kd_mbr.h"

struct KdOnlineLDecoderOpts: public kaldi::LatticeFasterDecoderConfig
{
    int32 batch_size = 27;       // number of features decoded in one go
    int32 inter_utt_sil = 50;    // minimum silence (#frames) to trigger end of utterance
    int32 max_utt_len_  = 1500;  // if utt. is longer, we accept shorter silence as utt. separators
    int32 update_interval = 3;   // beam update period in # of frames
    float beam_update = 0.01;    // rate of adjustment of the beam
    float max_beam_update = 0.05;// maximum rate of beam adjustment
};

struct KdOnlineStatus
{
    int state = KD_STATE_NORMAL;
    int max_frame = -1;
    int word_count = -1;
    QString last_word;
};

class KdOnlineLDecoder : public KdLatticeDecoder
{
public:
    // "sil_phones" - the IDs of all silence phones
    KdOnlineLDecoder(fst::Fst<fst::StdArc> &fst, KdOnlineLDecoderOpts &opts,
                     QVector<int> sil_phones, kaldi::TransitionModel &trans_model);


    int Decode(kaldi::DecodableInterface *decodable);

    void createStates(kaldi::Lattice *ofst);
    void RawLattice(kaldi::Lattice *ofst);
    void MakeLattice(kaldi::CompactLattice *ofst);

    QVector<BtWord> getResult(kaldi::CompactLattice *out_fst,
                              QVector<QString> lexicon);

    void HaveSilence(QVector<BtWord> result);

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
    KdOnlineStatus status;
    float effective_beam_; // the currently used beam
    int   uframe;          // reset on ResetDecoder(utterance)
    clock_t start_t;
};
#endif // BT_LAT_ONLINE

#endif // KD_ONLINE_DECODER_H
