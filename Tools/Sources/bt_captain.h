#ifndef BT_CAPTAIN_H
#define BT_CAPTAIN_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QVector>
#include <QTime>
#include <QTextStream>

#include "config.h"
#include "kd_mbr.h"
#include "bt_network.h"

#define BT_TIME_NOW QTime::currentTime().toString("hh:mm:ss")
#define BT_HISTORY_UPDATE 300  // update interval in ms
#define BT_HISTORY_SIZE   9000 // interval in ms that preserved
#define BT_HISTORY_LEN    10   // maximum number of words in history
#define BT_MAXSYNC_DIFF   1.0  // maximum error on frame_num and local captain time

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
    void parse(QVector<BtWord> in_words, uint max_frame);

    BtNetwork *net;
public slots:
    void flush(); //flush current utterance into history
    void shiftHistory();

private:
    void  exec(QString word);
    void  addXBuf(BtWord word); //add to the lest of exec
    float getConf(BtWord word);
    void  addWord(BtWord word, int id);
    QString getWordFmt(BtHistory word);
    QString getConfColor(float conf);
    void  writeResult();
    void  syncFrame(uint max_frame);

    QVector<BtHistory>  history;
    QVector<BtHistory>  current;
    QTimer    *time_shifter;
    float      start_treshold;
    QString    x_buf; //exec buf
};

#endif // BT_CAPTAIN_H
