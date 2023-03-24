#ifndef AB_WAV_READER_H
#define AB_WAV_READER_H

#include "ab_recorder.h"
#include "ab_mbr_base.h" //For BtWord
#include "backend.h"
#include "ab_stat.h"

class AbWavReader
{
public:
    AbWavReader(int16_t *buffer, int sample_count);
    ~AbWavReader();

    void read(QString filename);

    QString category;
    double wave_time;
    double power_dB;

private:
    void readWav();
    void readWavHeader();

    int16_t     *cy_buf;
    int          buf_size;
    QFile       *file;
};
#endif // AB_WAV_READER_H
