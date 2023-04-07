#ifndef ABAUDIO_H
#define ABAUDIO_H

#include <QObject>
#include "ab_recorder.h"
#include "ab_wav_writer.h"
#include "ab_wav_reader.h"
#include "ab_stat.h"

class AbAudio : public QObject
{
    Q_OBJECT
public:
    explicit AbAudio(AbStat *st, QObject *ui,
                     QObject *parent = nullptr);
    void record();
    void updateAudioParam(QString filename);

signals:
    void setStatus(int status);

private slots:
    void updateTime(int percent);
    void writeWav();
    void breakTimeout();

private:
    double calcPower(int16_t *buffer, int len);
    QString getRandPath(QString category);
    QString getFileName(QVector<AbWord> words, QString category);
    QString wordToId(QVector<AbWord> result);
    void printWords(QVector<AbWord> words);
    void checkCategoryExist();
    QString idsToWords(QVector<int> ids);

    AbRecorder *rec;
    AbWavWriter *wav_wr;
    AbWavReader *wav_rd;
    AbStat *stat;
    QObject *root;//root qml object
    QObject *editor;//editor qml object
    QTimer *read_timer;
    QString wav_path;
};

#endif // ABAUDIO_H
