#ifndef KD_LATTICE_FUNCTIONS_H
#define KD_LATTICE_FUNCTIONS_H

#include <QDebug>
#include "bt_config.h".
#include "kd_token2.h"

#include <vector>
#include <map>

#include "base/kaldi-common.h"
#include "hmm/posterior.h"
#include "hmm/transition-model.h"
#include "lat/kaldi-lattice.h"
#include "kd_lattice.h"

// A representable float near .001.
#define KD_KDELTA fst::kDelta

//DeterminizeLatticePrunedOptions
struct KdDetOpt
{
    float delta = KD_KDELTA; // A small offset used to measure equality of weights.
    int max_mem = -1; // If >0, determinization will fail and return false
    // when the algorithm's (approximate) memory consumption crosses this threshold.
    int max_loop = -1; // If >0, can be used to detect non-determinizable input
    // (a case that wouldn't be caught by max_mem).
    int max_states = -1 ;
    int max_arcs = -1;
    float retry_cutoff = 0.5;
};

struct KdPrunedOpt
{
  // delta: a small offset used to measure equality of weights.
  float delta = KD_KDELTA;
  // determinization will fail when the
  // algorithm's (approximate) memory consumption crosses this threshold.
  int max_mem = 50000000;
  // do a first pass determinization on both phones and words.
  bool phone_determinize = true;
  // do a second pass determinization on words only.
  bool word_determinize = true;
  // push and minimize after determinization if true, .
  bool minimize = false;
};

void kd_latticeGetTimes(KdLattice *lat, std::vector<int> *times);
bool kd_PruneLattice(float beam, KdLattice *lat);
bool kd_detLatPhonePrunedW(kaldi::TransitionModel &trans_model,
                           KdLattice *ifst, double beam,
                           kaldi::CompactLattice *ofst, KdPrunedOpt opts);
bool kd_detLatPhonePruned(kaldi::TransitionModel &trans_model,
                          KdLattice *ifst, double beam,
                          kaldi::CompactLattice *ofst, KdPrunedOpt opts);
/** first pass determinization with phone symbols inserted
    at phone boundary. It uses a transition model to work out the transition-id
    to phone map. First, phones will be inserted into the word level lattice.
    Second, determinization will be applied on top of the phone + word lattice.
    Finally, the inserted phones will be removed, converting the lattice back to
    a word level lattice. The output lattice of this pass is not deterministic,
    since we remove the phone symbols as a last step. It is supposed to be
    followed by another pass of determinization at the word level. It could also
    be useful for some other applications such as fMLLR estimation, confidence
    estimation, discriminative training, etc.
*/
bool kd_DetLatFirstPass(kaldi::TransitionModel &trans_model,
                        double beam, KdLattice *fst, KdDetOpt *opts);
bool kd_detLatPruned(KdLattice &ifst, double beam,
                     KdLattice *ofst, KdDetOpt opts);
bool kd_detLatPruned(KdLattice &ifst, double beam,
                     kaldi::CompactLattice *ofst, KdDetOpt opts);


#endif // KD_LATTICE_FUNCTIONS_H
