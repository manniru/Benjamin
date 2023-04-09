#ifndef AB_PHONEME_H
#define AB_PHONEME_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include "config.h"
#include "backend.h"

class AbPhoneme: public QObject
{
    Q_OBJECT
public:
    explicit AbPhoneme(QObject *parent = nullptr);
    ~AbPhoneme();

     QString getPhoneme(QString word);
public slots:

signals:

private:
     void loadPhoneme();

     QVector<QString> words;
     QVector<QString> phonemes;
};

#endif // AB_PHONEME_H
