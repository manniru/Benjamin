#ifndef MM_BAR_H
#define MM_BAR_H

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

typedef struct MmLabel
{
    QString     content;
    MmProperty properties;
}MmLabel;


class MmBar : public QObject
{
    Q_OBJECT
public:
    explicit MmBar(QObject *root, QObject *parent = nullptr);

private slots:
    void executeCommand(QString action);
    void updateLabels();

private:
    void loadLabels(QString path, QObject *list_ui);
    void parseProps(QString raw, MmProperty *properties);
    void showLabels(QVector<MmLabel> labels, QObject *list_ui);
    void proccessFile(QString data, QObject *list_ui);

    QObject *left_bar_ui;
    QObject *right_bar_ui;

    QTimer *timer;
};

#endif // MM_BAR_H
