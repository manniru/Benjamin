#ifndef BT_CONFIDENE_H
#define BT_CONFIDENE_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QVector>

#include "bt_config.h"

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

    QVector<QString> lexicon;
    QString utterance;

    double sum_conf;
    double sum_det;
    double avg_conf;
    double avg_det;
};

#endif // BT_CONFIDENE_H
