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
    MmParser(QObject *root);

    void updateProps(QString raw, MmProperty *properties);
    void parse(QString data, QVector<MmLabel> *out);

private:
    int parseProps(QString data, int s_index);
    int readActions(QString raw, MmProperty *properties);

    MmProperty c_property;
    QObject   *ui;
};

#endif // MM_PARSER_H
