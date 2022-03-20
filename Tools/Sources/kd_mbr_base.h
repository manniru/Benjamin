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

#endif // KD_MBR_BASE_H