#ifndef AB_MANAGER_H
#define AB_MANAGER_H

#include <QObject>
#include <thread>         // std::thread

#include "ab_recorder.h"
#include "ab_wav_writer.h"
#include "ab_wav_reader.h"
#include "ab_stat.h"

typedef struct AbRecParam
{
    QString category = "online";
    QString words = "<One> <Roger> <Spotify>";
    QString stat = "One: 10 Two: 13 ...\nAlpha: 22  ...";
    QString address;
    QString focusword = "<empty>";
    QString wordlist;
    QString wordstat;
    QString difwords;
    QString autocomp;
    QString meanvar;
    qreal   count = 0;
    qreal   total_count = 100;
    qreal   elapsed_time = 0;
    qreal   status = AB_STATUS_STOP;
    qreal   rec_time = 3;
    qreal   num_words = 3;
    qreal   pause_time = 1;
    qreal   key = 0;
    qreal   power = 0;
    qreal   verifier = 0;
    qreal   loadsrc = 0;
    qreal   delfile = 0;
    qreal   playkon = 0;
}AbRecParam;

class AbManager : public QObject
{
    Q_OBJECT
public:
    explicit AbManager(QObject *parent = nullptr);
    ~AbManager();

    void record();
    void swapParams();
    void readWave(QString filename);
    void writeWordList();
    QString readWordList();
    void copyToOnline(QString filename);
    void delWordSamples();

    AbRecParam params;
    AbRecParam p_backup;

signals:
    void startDecoding();
    void wordsChanged(QString total_words);
    void statusChanged(qreal status);
    void countChanged(qreal count);
    void timeChanged(qreal time);
    void powerChanged(qreal power);

    void pauseChanged(qreal time);
    void numWordChanged(qreal num_word);
    void recTimeChanged(qreal time);
    void categoryChanged(QString category);
    void totalCountChanged(qreal total_count);

private slots:
    void writeWav();
    void breakTimeout();
    void updateTime(int percent);

private:
    QString getRandPath(QString category);
    void loadWordList();
    QString getFileName(QVector<AbWord> words, QString category);
    QString wordToId(QVector<AbWord> result);
    void printWords(QVector<AbWord> words);
    QString idsToWords(QVector<int> ids);
    int wordToIndex(QString word);

    QStringList lexicon;
    AbRecorder *rec;
    AbWavWriter *wav_wr;
    AbWavReader *wav_rd;
    QTimer *read_timer;
    QString wav_path;
    QStringList  word_list;
    int r_counter;
};

double calcPower(int16_t *buffer, int len);

#endif // AB_MANAGER_H
