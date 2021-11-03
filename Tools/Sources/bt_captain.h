#ifndef BT_CAPTAIN_H
#define BT_CAPTAIN_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QVector>
#include <QTime>

#include "bt_config.h"

typedef struct BtWord
{
    QString word;
    double  time;
    double  start;
    double  end;
    double  conf;
}BtWord;

#define BT_TIME_NOW QTime::currentTime().toString("hh:mm:ss")

class BtCaptain : public QObject
{
    Q_OBJECT
public:
    explicit BtCaptain(QObject *parent = nullptr);
    void parse(QVector<BtWord> in_words);
    bool isValidUtterance();
    void printWords(QString words);
    QString getUtterance();

private:
    void processUtterance(BtWord word);
    double  getAvgConfidence();
    double  getAvgDetection();
    void    addWord(BtWord word);
    void writeBarResult();
    bool isValidTime(BtWord word);
    int  lastWordIndex(QVector<BtWord> in_words);
    int  lastWordIndex(double min, double max,
                       QVector<BtWord> in_words);
    void shiftHistory();
    void printConf(QVector<BtWord> in_words);

    QVector<BtWord>  history;
    QVector<BtWord>  words;  //words with conf>KAL_HARD_TRESHOLD
    QString utterance;
    BtWord  lastword;
};

#endif // BT_CAPTAIN_H
