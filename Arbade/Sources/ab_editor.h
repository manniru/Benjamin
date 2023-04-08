#ifndef ABEDITOR_H
#define ABEDITOR_H

#include <QObject>
#include "ab_stat.h"

class AbEditor : public QObject
{
    Q_OBJECT
public:
    explicit AbEditor(QObject *ui, QObject *parent = nullptr);

    void updateStat();
    void statAll();
    AbStat *stat;

signals:

private slots:
    void wordAdded(int id);
    void wordAddedRec(int id);
    void changeWord(int id, QString text);
    void saveProcess();
    void resetProcess();
    void writeWordList();
    void timerTimeout();
    void recRemove(int id);

private:
    void enableButtons();
    QString getUiWordList();
    QString getDif();

    QTimer *timer;
    QObject *root;    // root qml object
    QObject *editor;  // editor qml object
    QObject *buttons; // buttons qml object
    QObject *rec_list; // rec list qml object
    QVector<QObject *> editor_lines;
    QVector<QObject *> rec_lines;
};

#endif // ABEDITOR_H
