#ifndef AB_WRONG_H
#define AB_WRONG_H

#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>
#include "ab_editor.h"
#include "ab_audio.h"

class AbWrong : public QObject
{
    Q_OBJECT
public:
    explicit AbWrong(AbStat *st, QObject *ui,
                     QObject *parent = nullptr);

private slots:
    void generateWrongForms();
    void processKey(int key);

private:
    void addForm(QString w_word, QString w_path, QString shortcut);
    QVector<QString> createList(QString in);
    QString idToWord(QString filename, QString id);

    QVector<QString> w_shortcut;
    QVector<QString> w_word;
    QVector<QString> w_path;

    QObject *root;       // root qml object
    QObject *query; // query qml object
    AbStat *stat;
};

#endif // AB_WRONG_H
