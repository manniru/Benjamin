#ifndef BT_CONFIDENE_H
#define BT_CONFIDENE_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QVector>

#include "bt_config.h"

#define  BT_CONF_TIMEOUT 5000

typedef struct BtWord
{
    QString word;
    double  time;
    double  conf;
}BtWord;

class BtConfidence : public QObject
{
    Q_OBJECT
public:
    explicit BtConfidence(QObject *parent = nullptr);
    void parseConfidence();
    bool isValidUtterance();
    void printWords(QString words);
    QString getUtterance();

private:
    QString processLine(QString line);
    double  getAvgConfidence();
    double  getAvgDetection();
    void parseWords(QString filename);
    void writeConfidence(QVector<QString> lines);
    bool isValidTime(QString word, double start, double end);
    int  isLastWord(QString word, double middle);
    void shiftHistory();

    QVector<QString> lexicon;
    QVector<BtWord>  history;
    QVector<BtWord>  words;
    QString utterance;
    BtWord  lastword;
};

#endif // BT_CONFIDENE_H
