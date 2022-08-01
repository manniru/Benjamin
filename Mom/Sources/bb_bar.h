#ifndef BB_BAR_H
#define BB_BAR_H

#include <QObject>
#include <QQmlProperty>
#include <QVector>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include "bb_config.h"

typedef struct BpbProperty
{
    QString     background_color;
    QString     label_color;
    QString     underline_color;
    bool        have_underline;
    QString     action;
}BpbProperty;

typedef struct BpbLabel
{
    QString     content;
    BpbProperty properties;
}PbLabel;


class BbBar : public QObject
{
    Q_OBJECT
public:
    explicit BbBar(QObject *root, QObject *parent = nullptr);

private slots:
    void executeCommand(QString action);
    void updateLabels();

private:
    void loadLabels(QString path, QObject *list_ui, bool reverse=false);
    void updateProperty(QString rawProperty, BpbProperty *properties);
    void showLabels(QVector<BpbLabel> labels, QObject *list_ui, bool reverse);

    // Log function
    void logError(QString message);

private:
    QObject *left_bar_ui;
    QObject *right_bar_ui;

    QTimer *timer;
};

#endif // BB_BAR_H
