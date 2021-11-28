#ifndef KD_FASTER_TOKEN_H
#define KD_FASTER_TOKEN_H

#include "decoder/lattice-faster-decoder.h"

class KdToken2
{
public:
  using ForwardLinkT = kaldi::decoder::ForwardLink<KdToken2>;

  // tot_cost is the total (LM + acoustic) cost from the beginning of the
  // utterance up to this point.
  float tot_cost;
  float extra_cost; // always >= 0

  ForwardLinkT *links;
  KdToken2 *next;

  KdToken2(float tot_cost, float extra_cost, ForwardLinkT *links, KdToken2 *next);
};

#endif // KD_FASTER_TOKEN_H
