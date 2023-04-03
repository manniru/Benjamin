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
    void addWord(int id);
    void changeWord(int id, QString text);
    void saveProcess();
    void resetProcess();
    void writeWordList();

private:
    void enableButtons();
    QString getUiWordList();
    QString getDif();

    QObject *root;    // root qml object
    QObject *editor;  // editor qml object
    QObject *buttons; // buttons qml object
    QVector<QObject *> word_lines;
};

#endif // ABEDITOR_H
