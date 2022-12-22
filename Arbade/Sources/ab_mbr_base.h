#ifndef AB_MBR_BASE_H
#define AB_MBR_BASE_H

#include <QFile>
#include <QDebug>

typedef struct AbWord
{
    QString word;
    double  time;
    double  start;
    double  end;
    double  conf;
    int     is_final;
    int     word_id;
    int     stf; //start frame (used in enn)
}AbWord;

void ab_printResult(QVector<AbWord> result);

#endif // AB_MBR_BASE_H
