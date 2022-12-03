#ifndef BT_MBR_BASE_H
#define BT_MBR_BASE_H

#include <QString>
#include <QFile>

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
    int     word_id;
    int     stf; //start frame (used in enn)
}BtWord;

void bt_printResult(QVector<BtWord> result);
void bt_writeBarResult(QVector<BtWord> result);

#endif // BT_MBR_BASE_H
