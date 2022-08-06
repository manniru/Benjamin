#ifndef MM_PARSER_H
#define MM_PARSER_H

#include <QObject>
#include <QQmlProperty>
#include <QVector>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include "mm_config.h"
#include "mm_label.h"

class MmParser
{
public:
    MmParser();

    void updateProps(QString raw, MmProperty *properties);
    void proccessFile(QString data);

    QVector<MmLabel> labels;

private:
    int parseProps(QString data, int s_index);

    MmProperty c_property;
};

#endif // MM_PARSER_H
