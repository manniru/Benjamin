#ifndef BT_WAV_WRITER_H
#define BT_WAV_WRITER_H

#include <QTimer>
#include <QDebug>

#include "bt_recorder.h"
#include "bt_mbr_base.h" //For BtWord

class BtWavWriter
{
public:
    BtWavWriter(int16_t *buffer, int sample_count);
    ~BtWavWriter();

    void write(QVector<BtWord> result);
    void readWordList();
    QString wordToId(QVector<BtWord> result);
    void setCategory(QString cat);

private:
    void writeWav();
    void writeWavHeader(int len);
    QString calcFileName(QVector<BtWord> result);
    bool isSleep();

    int16_t *cy_buf;
    int buf_size;

    QFile       *file;
    QString      category;
    QStringList  word_list;
    QStringList  exemption_list;
};

#endif // BT_WAV_WRITER_H
