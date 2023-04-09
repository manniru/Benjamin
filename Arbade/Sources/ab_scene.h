#ifndef ABSCENE_H
#define ABSCENE_H

#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>
#include "ab_editor.h"
#include "ab_audio.h"

class AbScene : public QObject
{
    Q_OBJECT
public:
    explicit AbScene(QObject *ui, QObject *parent = nullptr);

private slots:
    void startPauseV();
    void copyFile();
    void deleteFile();
    void breakTimeout();
    void processKey(int key);
    void setStatus(int status);
    void updateStatus(int status);
    void verifierChanged();
    void setCategory();
    void setDifWords();
    void setFocusWord(int focus_word);
    void qmlCreated();

private:
    void fillRecParams();
    void updateCategories();
    void updateStat();
    void loadAddress();
    void setCount(int cnt);
    void readQmlProperties();

    QObject *root;       // root qml object
    QObject *qml_editor; // editor qml object
    AbAudio *audio;
    AbEditor  *editor;
    QTimer *break_timer;
    QStringList unverified_list;
    int stat_all = 0;
};

#endif // ABSCENE_H
