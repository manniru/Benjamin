#ifndef ABTESTERRORS_H
#define ABTESTERRORS_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include "config.h"
#include "backend.h"

class AbTestErrors : public QObject
{
    Q_OBJECT
public:
    explicit AbTestErrors(QObject *ui,
                          QObject *parent = nullptr);

signals:

private slots:
    void loadWordErrors();
    void timerTimeout();

private:
    void readWordErrors();
    void addWord(QString word, int count, QString phoneme);

    QObject *root;     // root qml object
    QObject *t_err_qml; // test errors in qml
    QObject *t_word_edit; // test word edit in qml
    QVector<QObject *> editor_lines;
    QVector<int> word_errors;
    QVector<QString> words;
    QTimer *timer_editor;
    void clearEditor();
};

#endif // ABTESTERRORS_H
