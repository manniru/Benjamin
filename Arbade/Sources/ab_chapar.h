#ifndef AB_CHAPAR_H
#define AB_CHAPAR_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>

#include "ab_recorder.h"
#include "ab_wav_writer.h"
#include "backend.h"

class AbChapar : public QObject
{
    Q_OBJECT
public:
    explicit AbChapar(QObject *parent = nullptr);
    ~AbChapar();

    void record(int count, QString category);

signals:
    void startDecoding();

private slots:
    void writeWav();

private:
    QString getRandPath(QString category);
    void readWordList();
    QString getFileName(QVector<AbWord> words, QString category);
    QString wordToId(QVector<AbWord> result);
    void printWords(QVector<AbWord> words);

    QStringList lexicon;
    AbRecorder *rec;
    AbWavWriter *wav;
    QString wav_path;
    QStringList  word_list;
    int gui_len;
    int gui_pause;
    int r_counter;
};

#endif // AB_CHAPAR_H
