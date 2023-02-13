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
    QString focus_word = "<empty>";
    QString word_list;
    QString dif_words;
    qreal   count = 0;
    qreal   total_count = 100;
    qreal   status = AB_STATUS_STOP;
    qreal   rec_time = 3;
    qreal   num_words = 3;
    qreal   pause_time = 1;
    qreal   verifier = 0;
}AbRecParam;

class AbManager : public QObject
{
    Q_OBJECT
public:
    explicit AbManager(QObject *ui, QObject *parent = nullptr);
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

    void pauseChanged(qreal time);
    void categoryChanged(QString category);

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
    void setStatus(int status);

    QObject* root;//root qml object
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
