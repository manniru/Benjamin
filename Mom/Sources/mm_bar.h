#ifndef MM_BAR_H
#define MM_BAR_H

#include <QObject>
#include "mm_config.h"
#include "mm_parser.h"

class MmBar : public QObject
{
    Q_OBJECT
public:
    explicit MmBar(QObject *root, QObject *parent = nullptr);

private slots:
    void executeCommand(QString action);
    void loadLabels();

private:
    void parseProps(QString raw, MmProperty *properties);
    void showLabels(QVector<MmLabel> labels, QObject *list_ui);
    void proccessFile(QString path, QObject *obj);

    QObject *left_bar;
    QObject *right_bar;

    QTimer   *timer;
    MmParser *parser;
};

#endif // MM_BAR_H
