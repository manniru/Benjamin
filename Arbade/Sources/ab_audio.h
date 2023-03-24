#ifndef ABAUDIO_H
#define ABAUDIO_H

#include <QObject>
#include "ab_recorder.h"
#include "ab_wav_writer.h"
#include "ab_wav_reader.h"

class AbAudio : public QObject
{
    Q_OBJECT
public:
    explicit AbAudio(QObject *ui, QObject *parent = nullptr);
    void record();
    void readWave(QString filename);
    void parseLexicon();

    QStringList lexicon;

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
    QObject *root;//root qml object
    QTimer *read_timer;
    QString wav_path;
};

#endif // ABAUDIO_H
