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
#define BT_HISTORY_SIZE   6000 // interval in ms that preserved

typedef struct BtHistory
{
    QVector<QString> words;
    double  time;
    double  conf;
    int     is_final;
}BtHistory;

class BtCaptain : public QObject
{
    Q_OBJECT
public:
    explicit BtCaptain(QObject *parent = nullptr);
    bool isValidUtterance();

public slots:
    void parse(QVector<BtWord> in_words);
    void flush(); //flush current utterance into history
    void shiftHistory();

private:
    void exec(QString word);
    void addXBuf(BtWord word); //add to the lest of exec
    void addWord(BtWord word, int id);
    QString getWordFmt(BtHistory word);
    void writeResult();

    QVector<BtHistory>  history;
    QVector<BtHistory>  current;
    QTimer *time_shifter;
    float   start_treshold;
    QString x_buf; //exec buf
};

#endif // BT_CAPTAIN_H
