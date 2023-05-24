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
    void loadNext();

private slots:
    void moveToOnline();
    void generateWrongForms();
    void execWrongKey(int key);
    void deleteFile();
    void trashFile();
    void updateVerifier();

private:
    void checkOnlineExist();
    void addWrongForm(QString w_word, QString w_path, QString shortcut);
    QVector<QString> createWrongList(QString in);
    QString idToWord(QString filename, QString id);
    void    recRemove();
    int     getId();
    QString getNext();
    QString idsToWords(QVector<int> ids);

    QVector<QString> w_shortcut;
    QVector<QString> w_word;
    QVector<QString> w_path;

    QObject  *root;     // root qml object
    QObject  *query;    // query qml object
    AbEditor *editor;
    AbWavReader *wav_rd;
    int verify_id = 0;
};

#endif // AB_VERIFY_H
