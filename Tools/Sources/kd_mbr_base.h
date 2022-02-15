#ifndef KD_MBR_BASE_H
#define KD_MBR_BASE_H

#include <QString>
#include "kd_lattice.h"
#include "kd_lattice_compact.h"

struct KdMBRArc
{
    int word;
    int start_node;
    int end_node;
    float loglike;
};

typedef struct BtWord
{
    QString word;
    double  time;
    double  start;
    double  end;
    double  conf;
    int     is_final;
}BtWord;

struct KdMBROpt
{
  /// Boolean configuration parameter: if true, we actually update the hypothesis
  /// to do MBR decoding (if false, our output is the MAP decoded output, but we
  /// output the stats too (i.e. the confidences)).
  bool decode_mbr = true;
  /// Boolean configuration parameter: if true, the 1-best path will 'keep' the <eps> bins,
  bool print_silence = false; //MAYBE THIS WILL FIX EVERYTHING!
  ///////////////////////////////////////////////////////////////////
  ////////////////////////////FIX ME/////////////////////////////////
  ///////////////////////////////////////////////////////////////////
};

#endif // KD_MBR_BASE_H
