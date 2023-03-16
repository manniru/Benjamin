#ifndef ABSCENE_H
#define ABSCENE_H

#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>
#include "ab_manager.h"
#include "ab_stat.h"

class AbScene : public QObject
{
    Q_OBJECT
public:
    explicit AbScene(QObject *ui, QObject *parent = nullptr);

private slots:
    void loadsrc();
    void copyFile();
    void deleteFile();
    void breakTimeout();
    void loadWordList();
    void processKey(int key);
    void setStatus(int status);
    void setVerifier(int verifier);
    void saveWordList(QString word_list);
    void setCategory(QString cat);
    void setDifWords(QString difwords);
    void setFocusWord(int focus_word);

private:
    void fillRecParams();
    void updateCategories();
    void updateStat();
    void loadAddress();
    void setCount(int cnt);
    void readQmlProperties();

    QObject* root;//root qml object
    AbManager *man;
    AbRecParam *rec_params;
    QTimer *break_timer;
    QStringList unverified_list;
    int catmode = 0;
};

#endif // ABSCENE_H
