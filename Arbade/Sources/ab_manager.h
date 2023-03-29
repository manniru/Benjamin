#ifndef AB_MANAGER_H
#define AB_MANAGER_H

#include <QObject>
#include <thread>         // std::thread
#include "ab_stat.h"
#include "ab_audio.h"

class AbManager : public QObject
{
    Q_OBJECT
public:
    explicit AbManager(QObject *ui, QObject *parent = nullptr);
    ~AbManager();

    void record();
    void writeWordList();
    void delWordSamples();
    void readWave(QString filename);
    void copyToOnline(QString filename);
    QString readWordList();
    QString idToWord(int id);

signals:
    void startDecoding();
    void pauseChanged(float time);
    void categoryChanged(QString category);

private slots:
    void setStatus(int status);
    void deleteSample(QString sample);

private:
    void loadWordList();
    int wordToIndex(QString word);
    void deleteFile(QString path);

    AbAudio *audio;
    AbStat  *stat;
    QObject *root;//root qml object
    QObject *editor;//editor qml object
    int r_counter;
};

#endif // AB_MANAGER_H
