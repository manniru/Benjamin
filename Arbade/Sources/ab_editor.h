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
    void createList();

    AbStat  *stat;
    QThread *stat_thread;

public slots:
    void recRemove(int id);

signals:
    void create(QString category);

private slots:
    void wordAdded(int id);
    void wordAddedRec(int id);
    void changeWord(int id, QString text);
    void saveProcess();
    void resetProcess();
    void writeWordList();
    void timerTimeout();

private:
    void updateStatAll();
    void updateStatCat();
    void enableButtons();
    QString getUiWordList();
    QString getDif();

    QTimer *timer;
    QObject *root;     // root qml object
    QObject *editor;   // editor qml object
    QObject *buttons;  // buttons qml object
    QObject *rec_list; // rec list qml object
    QObject *message;  // messege qml object
    QVector<QObject *> editor_lines;
    QVector<QObject *> rec_lines;
};

#endif // ABEDITOR_H
