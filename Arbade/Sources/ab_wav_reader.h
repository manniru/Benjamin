#ifndef AB_WAV_READER_H
#define AB_WAV_READER_H

#include "ab_recorder.h"
#include "ab_mbr_base.h" //For BtWord
#include "backend.h"
#include "ab_stat.h"

class AbWavReader
{
public:
    AbWavReader();
    ~AbWavReader();

    double getPower(QString filename);

    QString category;
    double wave_time;

private:
    double calcPower();
    void readHeader();

    QFile       *wav_file;
};
#endif // AB_WAV_READER_H
