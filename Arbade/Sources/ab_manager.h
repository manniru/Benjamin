#ifndef AB_MANAGER_H
#define AB_MANAGER_H

#include <QObject>
#include <thread>         // std::thread
#include <QTimer>

#include "ab_recorder.h"
#include "ab_wav_writer.h"
#include "backend.h"

typedef struct AbRecordParam
{
    QString category = "sag";
    QString words = "<One> <Roger> <Spotify>";
    QString stat = "One: 10 Two: 13 ...\nAlpha: 22  ...";
    qreal   count = 0;
    qreal   total_count = 100;
    qreal   elapsed_time = 0;
    qreal   status = AB_STATUS_STOP;
    qreal   rec_time = 3;
    qreal   num_words = 3;
    qreal   pause_time = 1;
    qreal   key = 0;
}AbRecordParam;

class AbManager : public QObject
{
    Q_OBJECT
public:
    explicit AbManager(QObject *parent = nullptr);
    ~AbManager();

    void record();

    AbRecordParam params;

signals:
    void startDecoding();
    void wordsChanged(QString total_words);
    void statusChanged(qreal status);
    void countChanged(qreal count);
    void timeChanged(qreal time);

private slots:
    void writeWav();
    void breakTimeout();
    void updateTime(int percent);

private:
    QString getRandPath(QString category);
    void readWordList();
    QString getFileName(QVector<AbWord> words, QString category);
    QString wordToId(QVector<AbWord> result);
    void printWords(QVector<AbWord> words);

    QStringList lexicon;
    AbRecorder *rec;
    AbWavWriter *wav;
    QTimer *read_timer;
    QString wav_path;
    QStringList  word_list;
    int r_counter;
};

#endif // AB_MANAGER_H
