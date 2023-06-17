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
    void addWord(QString word, QString count, QString wrong);
    void clearEditor();
    void addToLerStat(QString word1, QString word2);
    void updateLerMean();
    void resetLerVars();
    void sortLer();

    QObject *root;     // root qml object
    QObject *ler_qml; // lexicon error rate form in qml
    QVector<int> ler; // lexicon error rate
    QVector<QString> words;
    QVector<QStringList> wrong_out; // wrong detected words
    QVector<QVector<int>> wrong_count; // number of wrong detected word
    QTimer *timer_editor;
    QVector<int> sorted_indices;
    QVector<QVector<int>> sorted_wr_indices; // sorted indices for each wrong list
    int mean; // mean ler
    int sum_ler; // reverse mean ler
};

QVector<int> sortAndGetIndices(const QVector<int>& data);

#endif // AB_LER_STAT_H
