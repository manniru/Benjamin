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
    void clearRecList();

    AbStat  *stat;
    QThread *stat_thread;
    QString  dif_wordlist;
    QVector<QObject *> editor_lines;
    QVector<QObject *> rec_lines;

public slots:
    void recRemove(int id, int f_focus=1);

signals:
    void create(QString category);

private slots:
    void wordAdded(int id);
    void wordAddedRec(int id);
    void changeWord(int id, QString text);
    void saveProcess();
    void resetProcess();
    void writeWordList();
    void timerEdTimeout();
    void timerRecTimeout();

private:
    void updateStatAll();
    void updateStatCat();
    void enableButtons();
    QString getUiWordList();
    QString getDif();

    QTimer *timer_editor;
    QTimer *timer_rec;
    QObject *root;     // root qml object
    QObject *editor;   // editor qml object
    QObject *buttons;  // buttons qml object
    QObject *rec_list; // rec list qml object
    QObject *message;  // messege qml object
};

#endif // ABEDITOR_H
