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
    void stop();

signals:
    void setStatus(int status);
    void reqUpdateStat();

private slots:
    void updateTime(int percent);
    void writeWav();
    void breakTimeout();

private:
    double calcPower(int16_t *buffer, int len);
    QString getRandPath(QString category);
    QString getFileName(QVector<AbWord> words, QString category);
    QString wordToId(QVector<AbWord> result);
    void showWords(QVector<AbWord> words);
    void checkCategoryExist();

    AbRecorder *rec;
    AbWavWriter *wav_wr;
    AbStat *stat;
    QObject *root;//root qml object
    QObject *editor;//editor qml object
    QTimer *read_timer;
    QString wav_path;
    QString total_words;
    int pause_while_break=0;
};

#endif // ABAUDIO_H
