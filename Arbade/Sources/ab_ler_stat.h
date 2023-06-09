#ifndef AB_LER_STAT_H
#define AB_LER_STAT_H
// Lexicon Error Rate Statistics

#include <QObject>
#include <QFile>
#include <QDebug>
#include "config.h"
#include "backend.h"

class AbLerStat : public QObject
{
    Q_OBJECT
public:
    explicit AbLerStat(QObject *ui,
                          QObject *parent = nullptr);

signals:

private slots:
    void loadLer();
    void timerTimeout();

private:
    void readLerFile();
    void addWord(QString word, int count, QString phoneme);
    void clearEditor();

    QObject *root;     // root qml object
    QObject *ler_qml; // test errors in qml
    QObject *word_edit; // test word edit in qml
    QVector<QObject *> editor_lines;
    QVector<int> ler; // lexicon error rate
    QVector<QString> words;
    QTimer *timer_editor;
    int mean;
};

#endif // AB_LER_STAT_H
