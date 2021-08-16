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
    double  start;
    double  end;
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

    QVector<BtWord>  words;

private:
    QString processLine(QString line);
    void parseWords(QString filename);
    void writeConfidence(QVector<QString> lines);

    QVector<QString> lexicon;
};

#endif // BT_CONFIDENE_H
