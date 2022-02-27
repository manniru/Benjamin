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
#define BT_HISTORY_UPDATE 300  // update interval in ms
#define BT_HISTORY_SIZE   6500 // interval in ms that preserved

class BtCaptain : public QObject
{
    Q_OBJECT
public:
    explicit BtCaptain(QObject *parent = nullptr);
    bool isValidUtterance();
    void printWords(QString words);

public slots:
    void parse(QVector<BtWord> in_words);
    void shiftHistory();

private:
    void    addWord(BtWord word);

    QVector<BtWord>  history;
    QString utterance;
    QTimer *time_shifter;
    float   start_treshold;
};

void bt_writeBarResult(QVector<BtWord> result);

#endif // BT_CAPTAIN_H
