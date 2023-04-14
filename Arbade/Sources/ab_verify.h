#ifndef AB_VERIFY_H
#define AB_VERIFY_H

#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>
#include "ab_editor.h"
#include "ab_audio.h"

class AbVerify : public QObject
{
    Q_OBJECT
public:
    explicit AbVerify(AbEditor *ed, QObject *ui,
                     QObject *parent = nullptr);
    QString wrongAll(QString file_path);

private slots:
    void moveToOnline();
    void generateWrongForms();
    void execWrongKey(int key);
    void deleteFile();
    void trashFile();

private:
    void checkOnlineExist();
    void addWrongForm(QString w_word, QString w_path, QString shortcut);
    QVector<QString> createWrongList(QString in);
    QString idToWord(QString filename, QString id);
    void recRemove();

    QVector<QString> w_shortcut;
    QVector<QString> w_word;
    QVector<QString> w_path;

    QObject *root;       // root qml object
    QObject *query; // query qml object
    AbEditor *editor;
};

#endif // AB_VERIFY_H
