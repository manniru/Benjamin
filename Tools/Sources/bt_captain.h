#ifndef BT_CAPTAIN_H
#define BT_CAPTAIN_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QVector>

#include "bt_config.h"
#include "bt_confidence.h"

class BtCaptain : public QObject
{
    Q_OBJECT
public:
    explicit BtCaptain(QObject *parent = nullptr);
    void parse();
    bool isValidUtterance();
    void printWords(QString words);
    QString getUtterance();

private:
    void processUtterance(BtWord word);
    double  getAvgConfidence();
    double  getAvgDetection();
    void    addWord(QString word, double middle, double conf);
    void writeBarResult();
    bool isValidTime(double start);
    int  isLastWord(QString word, double middle);
    int  lastWordIndex();
    int  lastWordIndex(double max_dist);
    void shiftHistory();

    QVector<BtWord>  history;
    QVector<BtWord>  words;  //words with conf>KAL_HARD_TRESHOLD
    QString utterance;
    BtWord  lastword;

    BtConfidence *conf;
};

#endif // BT_CAPTAIN_H
