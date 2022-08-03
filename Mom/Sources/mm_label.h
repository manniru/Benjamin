#ifndef MM_LABEL_H
#define MM_LABEL_H

#include <QObject>
#include <QQmlProperty>
#include <QVector>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include "mm_config.h"

typedef struct MmProperty
{
    QString     bg;
    QString     label_color;
    QString     underline_color;
    bool        have_underline;
    QString     action;
}MmProperty;

class MmLabel
{
public:
    MmLabel();

    void setVal(QString input);

    QString    val;
    MmProperty properties;
};

#endif // MM_LABEL_H
