#ifndef BT_CONFIDENE_H
#define BT_CONFIDENE_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QVector>

#include "bt_config.h"

typedef struct BtWord
{
    QString word;
    double  time;
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
    void parseWords(QString filename);
    void writeConfidence(QVector<QString> lines);
    void isValidWord(QString word, double start, double end, double conf);
    int  isLastWord(QString word, double middle);

    QVector<QString> lexicon;
    QVector<BtWord>  words;
    QString utterance;
    BtWord  lastword;

    double sum_conf;
    double sum_det;
    double avg_conf;
    double avg_det;
};

#endif // BT_CONFIDENE_H
