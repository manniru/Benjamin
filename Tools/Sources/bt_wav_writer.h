#ifndef BT_WAV_WRITER_H
#define BT_WAV_WRITER_H

#include <QTimer>
#include <QDebug>

#include "bt_recorder.h"
#include "bt_mbr_base.h" //For BtWord
#include "bt_state.h"

class BtWavWriter
{
public:
    BtWavWriter(BtCyclic *buffer, BtState *state);
    ~BtWavWriter();

    void write(QVector<BtWord> result, int len, int dbg_id);
    void writeEnn(QString path, int len);
    void readWordList();
    QString wordToId(QVector<BtWord> result);

private:
    void writeWav(int len);
    void writeWavHeader(int len);
    void copyToUnverified(QVector<BtWord> result, QString filename);
    bool isSleep();

    BtCyclic    *cy_buf;
    QFile       *file;
    BtState     *st;
    QStringList  word_list;
    QStringList  exemption_list;
};

#endif // BT_WAV_WRITER_H
