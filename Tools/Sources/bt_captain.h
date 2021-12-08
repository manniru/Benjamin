#ifndef BT_CAPTAIN_H
#define BT_CAPTAIN_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QVector>
#include <QTime>
#include <QTextStream>

#include "bt_config.h"
#include "kd_mbr.h"

#define BT_TIME_NOW QTime::currentTime().toString("hh:mm:ss")

class BtCaptain : public QObject
{
    Q_OBJECT
public:
    explicit BtCaptain(QObject *parent = nullptr);
    bool isValidUtterance();
    void printWords(QString words);
    QString getUtterance();

public slots:
    void parse(QVector<BtWord> in_words);

private:
    void processUtterance(BtWord word);
    double  getAvgConfidence();
    double  getAvgDetection();
    void    addWord(BtWord word);
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

void bt_writeBarResult(QVector<QString> result);
void bt_writeBarResult(QVector<BtWord> result);

#endif // BT_CAPTAIN_H
