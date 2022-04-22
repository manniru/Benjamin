#ifndef BT_MBR_BASE_H
#define BT_MBR_BASE_H

#include <QString>
#include <QFile>
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

void bt_writeBarResult(QVector<BtWord> result);

#endif // BT_MBR_BASE_H
