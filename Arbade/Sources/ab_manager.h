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
    QString category;
    QString word_list;
    QString dif_words;
    int   focus_word;
    int   count;
    int   total_count;
    int   status;
    int   num_words;
    int   verifier;
    float   rec_time;
    float   pause_time;
}AbRecParam;

class AbManager : public QObject
{
    Q_OBJECT
public:
    explicit AbManager(QObject *ui, QObject *parent = nullptr);
    ~AbManager();

    void record();
    void swapParams();
    void writeWordList();
    void delWordSamples();
    void readWave(QString filename);
    void copyToOnline(QString filename);
    QString readWordList();
    QString idToWords(int id);
    QString idsToWords(QVector<int> ids);

    AbRecParam params;
    AbRecParam p_backup;

signals:
    void startDecoding();

    void pauseChanged(float time);
    void categoryChanged(QString category);

private slots:
    void writeWav();
    void breakTimeout();
    void updateTime(int percent);

private:
    void loadWordList();
    void setStatus(int status);
    void printWords(QVector<AbWord> words);
    int wordToIndex(QString word);
    QString getRandPath(QString category);
    QString wordToId(QVector<AbWord> result);
    QString getFileName(QVector<AbWord> words, QString category);

    QObject* root;//root qml object
    QTimer *read_timer;
    AbRecorder *rec;
    AbWavWriter *wav_wr;
    AbWavReader *wav_rd;
    QString wav_path;
    QStringList lexicon;
    QStringList  word_list;
    int r_counter;
};

double calcPower(int16_t *buffer, int len);

#endif // AB_MANAGER_H
